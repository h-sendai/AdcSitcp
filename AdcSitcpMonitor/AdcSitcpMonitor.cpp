// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */

#include "AdcSitcpMonitor.h"

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::INPORT_ERROR;
using DAQMW::FatalType::HEADER_DATA_MISMATCH;
using DAQMW::FatalType::FOOTER_DATA_MISMATCH;
using DAQMW::FatalType::USER_DEFINED_ERROR1;
using DAQMW::FatalType::USER_DEFINED_ERROR2;

// Module specification
// Change following items to suit your component's spec.
static const char* adcsitcpmonitor_spec[] =
{
    "implementation_id", "AdcSitcpMonitor",
    "type_name",         "AdcSitcpMonitor",
    "description",       "AdcSitcpMonitor component",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, Hiroshi Sendai, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

AdcSitcpMonitor::AdcSitcpMonitor(RTC::Manager* manager)
    : DAQMW::DaqComponentBase(manager),
      m_InPort("adcsitcpmonitor_in",   m_in_data),
      m_in_status(BUF_SUCCESS),
      m_canvas(0),
      m_bin(4096),
      m_min(0.0),
      m_max(4096.0),
      m_event_byte_size(0),
      m_monitor_update_rate(30),
      m_use_adc_sum(false),
      m_debug(false)
{
    // Registration: InPort/OutPort/Service

    // Set InPort buffers
    registerInPort ("adcsitcpmonitor_in",  m_InPort);

    init_command_port();
    init_state_table();
    set_comp_name("ADCSITCPMONITOR");
}

AdcSitcpMonitor::~AdcSitcpMonitor()
{
}

RTC::ReturnCode_t AdcSitcpMonitor::onInitialize()
{
    if (m_debug) {
        std::cerr << "AdcSitcpMonitor::onInitialize()" << std::endl;
    }

    return RTC::RTC_OK;
}

RTC::ReturnCode_t AdcSitcpMonitor::onExecute(RTC::UniqueId ec_id)
{
    daq_do();

    return RTC::RTC_OK;
}

int AdcSitcpMonitor::daq_dummy()
{
    if (m_canvas) {
        m_canvas->Update();
        // daq_dummy() will be invoked again after 10 msec.
        // This sleep reduces X servers' load.
        sleep(1);
    }

    return 0;
}

int AdcSitcpMonitor::daq_configure()
{
    std::cerr << "*** AdcSitcpMonitor::configure" << std::endl;

    ::NVList* paramList;
    paramList = m_daq_service0.getCompParams();
    parse_params(paramList);

    return 0;
}

int AdcSitcpMonitor::parse_params(::NVList* list)
{

    std::cerr << "param list length:" << (*list).length() << std::endl;

    int len = (*list).length();
    for (int i = 0; i < len; i+=2) {
        std::string sname  = (std::string)(*list)[i].value;
        std::string svalue = (std::string)(*list)[i+1].value;

        std::cerr << "sname: " << sname << "  ";
        std::cerr << "value: " << svalue << std::endl;
        
        if (sname == "monitorUpdateRate") {
            if (m_debug) {
                std::cerr << "monitor update rate: " << svalue << std::endl;
            }
            char *offset;
            m_monitor_update_rate = (int)strtol(svalue.c_str(), &offset, 10);
        }
        if (sname == "drawHistogramChannel") {
            if (m_debug) {
                std::cerr << "drawHistogramChannel" << svalue << std::endl;
            }
            int ch = strtol(svalue.c_str(), NULL, 0);
            m_draw_ch_list.push_back(ch);
        }
        if (sname == "useAdcSum") {
            if (svalue == "yes" || svalue == "Yes" || svalue == "YES") {
                m_use_adc_sum = true;
                std::cerr << "m_use_adc_sum: true" << std::endl;

                m_bin = m_bin*8; // Monitor Component does not know window size until
                m_max = m_max*8; // read the 1st data.  8 is window size in this experiment.
            }
        }
        // If you have more param in config.xml, write here
    }

    return 0;
}

int AdcSitcpMonitor::daq_unconfigure()
{
    std::cerr << "*** AdcSitcpMonitor::unconfigure" << std::endl;
    if (m_canvas) {
        delete m_canvas;
        m_canvas = 0;
    }

    for (int ch = 0; ch < AdcSitcpPacket::N_CH; ch ++) {
        if (m_hist[ch]) {
            delete m_hist[ch];
            m_hist[ch] = 0;
        }
    }

    return 0;
}

int AdcSitcpMonitor::daq_start()
{
    std::cerr << "*** AdcSitcpMonitor::start" << std::endl;

    m_in_status  = BUF_SUCCESS;

    //////////////// CANVAS FOR HISTOS ///////////////////
    if (m_canvas) {
        delete m_canvas;
        m_canvas = 0;
    }

    int canvas_x  = 0;
    int canvas_y  = 0;
    int div_x     = 0;
    int div_y     = 0;
    int n_draw_ch = m_draw_ch_list.size();

    if (n_draw_ch < 2) {
        canvas_x = 400;
        canvas_y = 400;
        div_x = 1;
        div_y = 1;
    }
    else if (n_draw_ch == 2) {
        canvas_x = 800;
        canvas_y = 400;
        div_x = 2;
        div_y = 1;
    }
    else if (n_draw_ch <= 4) {
        canvas_x = 800;
        canvas_y = 800;
        div_x = 2;
        div_y = 2;
    }
    else if (n_draw_ch <= 8) {
        canvas_x = 800;
        canvas_y = 400;
        div_x = 4;
        div_y = 2;
    }
    else if (n_draw_ch == 9) {
        canvas_x = 800;
        canvas_y = 800;
        div_x = 3;
        div_y = 3;
    }
    else {
        canvas_x = 800;
        canvas_y = 800;
        div_x = 4;
        div_y = 4;
    }
    m_canvas = new TCanvas("c1", "histos", 0, 0, canvas_x, canvas_y);
    m_canvas->Divide(div_x, div_y);

    m_canvas->Update();

    ////////////////       HISTOS      ///////////////////
    for (int ch = 0; ch < AdcSitcpPacket::N_CH; ch ++) {
        if (m_hist[ch]) {
            delete m_hist[ch];
            m_hist[ch] = 0;
        }
        m_hist[ch] = new TH1F(Form("hist_%d", ch), Form("Ch_%d", ch), m_bin, m_min, m_max);
    }
    
// You may need the following ROOT options to improve your histogram graphics.
// These lines are not effective when compile because enclosed #if - #endif directive

#if 0
    int m_hist_bin = 100;
    double m_hist_min = 0.0;
    double m_hist_max = 1000.0;

    gStyle->SetStatW(0.4);
    gStyle->SetStatH(0.2);
    gStyle->SetOptStat("em");

    m_hist = new TH1F("hist", "hist", m_hist_bin, m_hist_min, m_hist_max);
    m_hist->GetXaxis()->SetNdivisions(5);
    m_hist->GetYaxis()->SetNdivisions(4);
    m_hist->GetXaxis()->SetLabelSize(0.07);
    m_hist->GetYaxis()->SetLabelSize(0.06);
#endif

    return 0;
}

int AdcSitcpMonitor::daq_stop()
{
    std::cerr << "*** AdcSitcpMonitor::stop" << std::endl;

    int draw_pos = 1;
    for (unsigned int i = 0; i < m_draw_ch_list.size(); i++) {
        m_canvas->cd(draw_pos);
        m_hist[m_draw_ch_list[i]]->Draw();
        draw_pos ++;
    }
    m_canvas->Update();

    reset_InPort();

    return 0;
}

int AdcSitcpMonitor::daq_pause()
{
    std::cerr << "*** AdcSitcpMonitor::pause" << std::endl;

    return 0;
}

int AdcSitcpMonitor::daq_resume()
{
    std::cerr << "*** AdcSitcpMonitor::resume" << std::endl;

    return 0;
}

int AdcSitcpMonitor::reset_InPort()
{
    int ret = true;
    while(ret == true) {
        ret = m_InPort.read();
    }

    return 0;
}

int AdcSitcpMonitor::fill_data(const unsigned char* mydata, const int size)
{
    m_asp.set_buf(mydata, size);
    if (! m_asp.is_adc_sitcp_data_packet()) {
        fatal_error_report(USER_DEFINED_ERROR1, "NOT A ADC_SITCP DATA TYPE");
    }

    // int trigger_count = m_asp.get_trigger_count(); // Not used in this application
    int window_size   = m_asp.get_window_size();

    if (m_use_adc_sum) { // Sum all adc value at each window
        for (int ch = 0; ch < AdcSitcpPacket::N_CH; ch ++) {
            int data = 0;
            for (int w = 0; w < window_size; w++) {
                int tmp_data = m_asp.get_data_at(ch, w);
                data += tmp_data;
            }
            m_hist[ch]->Fill(data);
        }
    }
    else {
        for (int ch = 0; ch < AdcSitcpPacket::N_CH; ch++) {
            for (int w = 0; w < window_size; w++) {
                int data = m_asp.get_data_at(ch, w);
                m_hist[ch]->Fill(data);
            }
        }
    }

    m_asp.reset_buf();

    return 0;
}

unsigned int AdcSitcpMonitor::read_InPort()
{
    /////////////// read data from InPort Buffer ///////////////
    unsigned int recv_byte_size = 0;
    bool ret = m_InPort.read();

    //////////////////// check read status /////////////////////
    if (ret == false) { // false: TIMEOUT or FATAL
        m_in_status = check_inPort_status(m_InPort);
        if (m_in_status == BUF_TIMEOUT) { // Buffer empty.
            if (check_trans_lock()) {     // Check if stop command has come.
                set_trans_unlock();       // Transit to CONFIGURE state.
            }
        }
        else if (m_in_status == BUF_FATAL) { // Fatal error
            fatal_error_report(INPORT_ERROR);
        }
    }
    else {
        recv_byte_size = m_in_data.data.length();
    }

    if (m_debug) {
        std::cerr << "m_in_data.data.length():" << recv_byte_size
                  << std::endl;
    }

    return recv_byte_size;
}

int AdcSitcpMonitor::daq_run()
{
    if (m_debug) {
        std::cerr << "*** AdcSitcpMonitor::run" << std::endl;
    }

    unsigned int recv_byte_size = read_InPort();
    if (recv_byte_size == 0) { // Timeout
        return 0;
    }

    check_header_footer(m_in_data, recv_byte_size); // check header and footer
    m_event_byte_size = get_event_size(recv_byte_size);

    /////////////  Write component main logic here. /////////////
    memcpy(&m_recv_data[0], &m_in_data.data[HEADER_BYTE_SIZE], m_event_byte_size);

    fill_data(&m_recv_data[0], m_event_byte_size);

    if (m_monitor_update_rate == 0) {
        m_monitor_update_rate = 1000;
    }

    // Draw histogram periodically
    unsigned long sequence_num = get_sequence_num();
    if ((sequence_num % m_monitor_update_rate) == 0) {
        int draw_pos = 1;
        for (unsigned int i = 0; i < m_draw_ch_list.size(); i++) {
            m_canvas->cd(draw_pos);
            m_hist[m_draw_ch_list[i]]->Draw();
            draw_pos ++;
        }
        m_canvas->Update();
    }
    /////////////////////////////////////////////////////////////

    inc_sequence_num();                      // increase sequence num.
    inc_total_data_size(m_event_byte_size);  // increase total data byte size

    return 0;
}

extern "C"
{
    void AdcSitcpMonitorInit(RTC::Manager* manager)
    {
        RTC::Properties profile(adcsitcpmonitor_spec);
        manager->registerFactory(profile,
                    RTC::Create<AdcSitcpMonitor>,
                    RTC::Delete<AdcSitcpMonitor>);
    }
};
