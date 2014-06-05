// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef ADCSITCPMONITOR_H
#define ADCSITCPMONITOR_H

#include "DaqComponentBase.h"

#include <arpa/inet.h> // for ntohl()

////////// ROOT Include files //////////
#include "TH1.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TApplication.h"

////////// Decoder //////////
#include "AdcSitcpPacket.h"
#include "logUtil.h"

using namespace RTC;

class AdcSitcpMonitor
    : public DAQMW::DaqComponentBase
{
public:
    AdcSitcpMonitor(RTC::Manager* manager);
    ~AdcSitcpMonitor();

    // The initialize action (on CREATED->ALIVE transition)
    // former rtc_init_entry()
    virtual RTC::ReturnCode_t onInitialize();

    // The execution action that is invoked periodically
    // former rtc_active_do()
    virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

private:
    TimedOctetSeq          m_in_data;
    InPort<TimedOctetSeq>  m_InPort;

private:
    int daq_dummy();
    int daq_configure();
    int daq_unconfigure();
    int daq_start();
    int daq_run();
    int daq_stop();
    int daq_pause();
    int daq_resume();

    int parse_params(::NVList* list);
    int reset_InPort();

    unsigned int read_InPort();
    //int online_analyze();
    int decode_data(const unsigned char* mydata);
    int fill_data(const unsigned char* mydata, const int size);

    BufferStatus m_in_status;

    ////////// ROOT Histogram //////////
    TCanvas *m_canvas;
    TH1F    *m_hist[AdcSitcpPacket::N_CH];
    int      m_bin;
    double   m_min;
    double   m_max;
    
    AdcSitcpPacket m_asp;
    std::vector <int> m_draw_ch_list;

    ////////// DAQ-Middleware Data Path //////////
    const static int MAX_BUFFER_SIZE = 1024*1024;
    unsigned char m_recv_data[MAX_BUFFER_SIZE];
    unsigned int  m_event_byte_size;
    int      m_monitor_update_rate;

    bool m_debug;
};


extern "C"
{
    void AdcSitcpMonitorInit(RTC::Manager* manager);
};

#endif // ADCSITCPMONITOR_H
