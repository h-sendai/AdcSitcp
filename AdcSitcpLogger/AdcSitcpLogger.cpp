// -*- C++ -*-
/*!
 * @file AdcSitcpLogger.cpp
 * @brief Event data logging component.
 * @date
 * @author Kazuo Nakayoshi <kazuo.nakayoshi@kek.jp>
 *
 * Copyright (C) 2011
 *     Kazuo Nakayoshi
 *     Electronics System Group,
 *     KEK, Japan.
 *     All rights reserved.
 *
 */

#include "AdcSitcpLogger.h"

// Possible fatal errors for this component
using DAQMW::FatalType::BAD_DIR;
using DAQMW::FatalType::CANNOT_OPEN_FILE;
using DAQMW::FatalType::CANNOT_WRITE_DATA;

// Module specification
static const char* mylogger_spec[] = {
    "implementation_id", "AdcSitcpLogger",
    "type_name",         "AdcSitcpLogger",
    "description",       "Event data Logging component",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

AdcSitcpLogger::AdcSitcpLogger(RTC::Manager* manager)
    : DAQMW::DaqComponentBase(manager),
      m_InPort("adcsitcplogger_in", m_in_data),
      m_isDataLogging(false),
      m_filesOpened(false),
      m_in_status(BUF_SUCCESS),
      m_update_rate(100),
      m_debug(false)
{
    // Registration: InPort/OutPort/Service
    registerInPort("adcsitcplogger_in", m_InPort);

    init_command_port();
    init_state_table();
    set_comp_name("AdcSitcpLogger");
}

AdcSitcpLogger::~AdcSitcpLogger()
{
}

RTC::ReturnCode_t AdcSitcpLogger::onInitialize()
{
    if (m_debug) {
        std::cerr << "AdcSitcpLogger::onInitialize()" << std::endl;
    }

    return RTC::RTC_OK;
}

RTC::ReturnCode_t AdcSitcpLogger::onExecute(RTC::UniqueId ec_id)
{
    daq_do();

    return RTC::RTC_OK;
}

int AdcSitcpLogger::daq_dummy()
{
    return 0;
}

int AdcSitcpLogger::daq_configure()
{
    std::cerr << "*** SampleLoggqer::configure" << std::endl;
    int ret = 0;
    m_isDataLogging = false;

    ::NVList* list = m_daq_service0.getCompParams();
    parse_params(list);

    return ret;
}

int AdcSitcpLogger::parse_params(::NVList* list)
{
    int ret = 0;

    bool isExistParamLogging = false;
    bool isExistParamDirName = false;

    int length = (*list).length();
    for (int i = 0; i < length; i += 2) {
        if (m_debug) {
            std::cerr << "NVList[" << (*list)[i].name
                      << "," << (*list)[i].value << "]" << std::endl;
        }
        std::string sname  = (std::string)(*list)[i].value;
        std::string svalue = (std::string)(*list)[i + 1].value;
        if (sname == "eventByteSize") {
            unsigned int eventByteSize = atoi(svalue.c_str());
            set_event_byte_size(eventByteSize);
            std::cerr << "ventByteSize:"
                      << eventByteSize << std::endl;
        }
        if (sname == "isLogging") {
            toLower(svalue); // all characters of cvale are converted to lower.
            isExistParamLogging = true;
            if (svalue == "yes") {
                m_isDataLogging = true;
                fileUtils = new FileUtils();
                isExistParamLogging = true;
                std::cerr << "AdcSitcpLogger: Save to file: true\n";
            }
            else if (svalue == "no") {
                m_isDataLogging = false;
                isExistParamLogging = true;
                std::cerr << "AdcSitcpLogger: Save to file: false\n";
            }
        }

        if (sname == "monRate") {
            m_update_rate = atoi(svalue.c_str());
            if (m_debug) {
                std::cerr << "update rate:" << m_update_rate << std::endl;
            }
        }
    }

    if (m_isDataLogging) {
        for (int i = 0; i < length ; i += 2) {
            if (m_debug) {
                std::cerr << "NVList[" << (*list)[i].name
                          << "," << (*list)[i].value << "]" << std::endl;
            }
            std::string sname  = (std::string)(*list)[i].value;
            std::string svalue = (std::string)(*list)[i + 1].value;

            if (sname == "dirName") {
                isExistParamDirName = true;
                m_dirName = svalue;
                if (m_isDataLogging) {
                    std::cerr << "Dir name for data saving:"
                              << m_dirName << std::endl;
                    if (fileUtils->check_dir(m_dirName) != true) {
                        delete fileUtils;
                        fileUtils = 0;
                        std::cerr << "Can not open directory:"
                                  << m_dirName << std::endl;
                        fatal_error_report(BAD_DIR);
                    }
                }
            }

            if (sname == "runNumber") {
                unsigned int runNumber = atoi(svalue.c_str());
                set_run_number(runNumber);
                std::cerr << "Run Number:"
                          << runNumber << std::endl;
            }

            if (sname == "maxFileSizeInMegaByte") {
                m_maxFileSizeInMByte = strtoul(svalue.c_str(), NULL, 0);
                std::cerr << "Max File size(MByte):"
                          << m_maxFileSizeInMByte << std::endl;
            }
        }
    }

    return ret;
}

int AdcSitcpLogger::daq_unconfigure()
{
    std::cerr << "*** AdcSitcpLogger::unconfigure" << std::endl;
    if (m_isDataLogging) {
        delete fileUtils;
        if (m_debug) {
            std::cerr << "fileUtils deleted\n";
        }
        fileUtils = 0;
    }
    return 0;
}

int AdcSitcpLogger::daq_start()
{
    std::cerr << "*** AdcSitcpLogger::start" << std::endl;

    m_in_status = BUF_SUCCESS;
    m_filesOpened = false;
    unsigned int runNumber = m_daq_service0.getRunNo();
    if (m_debug) {
        std::cerr << "runNumber:" << runNumber << std::endl;
    }
    if (m_isDataLogging) {
        int ret = 0;
        fileUtils->set_run_no(runNumber);
        if (m_debug) {
            std::cerr << "m_maxFileSizeInMByte:"
                      << m_maxFileSizeInMByte << std::endl;
        }
        fileUtils->set_max_size_in_megaBytes(m_maxFileSizeInMByte);
        ret = fileUtils->open_file(m_dirName);
        if (ret < 0) {
            std::cerr << "### ERROR: AdcSitcpLogger: open file failed\n";
            fatal_error_report(CANNOT_OPEN_FILE);
        }
        else {
            std::cerr << "*** AdcSitcpLogger: file open succeed\n";
            m_filesOpened = true;
        }
    }
    return 0;
}

int AdcSitcpLogger::daq_stop()
{
    std::cerr << "*** AdcSitcpLogger::stop" << std::endl;

    if (m_isDataLogging && m_filesOpened) {
        if (m_debug) {
            std::cerr << "AdcSitcpLogger::stop: close files \n";
        }
        fileUtils->close_file();
    }

    reset_InPort();
    return 0;
}

int AdcSitcpLogger::daq_pause()
{
    std::cerr << "*** AdcSitcpLogger::pause" << std::endl;
    return 0;
}

int AdcSitcpLogger::daq_resume()
{
    std::cerr << "*** AdcSitcpLogger::resume" << std::endl;
    return 0;
}

int AdcSitcpLogger::reset_InPort()
{
    bool ret = true;
    while (ret) {
        ret = m_InPort.read();
        if (ret == true) {
            std::cerr << "m_in_data.data.length(): "
                      << m_in_data.data.length() << std::endl;
        }
    }
    if (m_debug) {
        std::cerr << "*** AdcSitcpLogger::InPort flushed\n";
    }
    return 0;
}

void AdcSitcpLogger::toLower(std::basic_string<char>& s)
{
    for (std::basic_string<char>::iterator p = s.begin(); p != s.end(); ++p) {
        *p = tolower(*p);
    }
}

int AdcSitcpLogger::daq_run()
{

    int event_byte_size = 0;
    bool ret = m_InPort.read();

    if (ret == true) {
        int block_byte_size = m_in_data.data.length();

        event_byte_size =
            block_byte_size - HEADER_BYTE_SIZE - FOOTER_BYTE_SIZE;
        if (m_debug) {
            std::cerr << "m_in_data.data.length:"
                      << m_in_data.data.length() << std::endl;
            std::cerr << "event_byte_size w/ header, fooger = "
                      << event_byte_size << std::endl;
        }

        if (event_byte_size == 0) {
            return 0;
        }

        if (check_header_footer(m_in_data, block_byte_size)) {
            //data header and footer were valid, do nothing
        }
    }
    else {
        if (check_trans_lock()) {
            if (m_debug) {
                std::cerr << "**** trans unlock\n";
            }
            set_trans_unlock();
        }
        return 0;
    }

    if (m_isDataLogging) {
        int ret = fileUtils->write_data((char *)&m_in_data.data[HEADER_BYTE_SIZE],
                                        event_byte_size);

        if (ret < 0) {
            std::cerr << "### AdcSitcpLogger: ERROR occured at data saving\n";
            fatal_error_report(CANNOT_WRITE_DATA);
        }
    }

    inc_total_data_size(event_byte_size);
    inc_sequence_num();

    if (m_debug) {
        unsigned long long seq_num = get_sequence_num();
        if (seq_num % m_update_rate == 0) {
            std::cerr << "AdcSitcpLogger: loop = " << seq_num << std::endl;
            std::cerr << "\033[A\r";
        }
    }

    return 0;
}

extern "C"
{
    void AdcSitcpLoggerInit(RTC::Manager* manager)
    {
        RTC::Properties profile(mylogger_spec);
        manager->registerFactory(profile,
                                 RTC::Create<AdcSitcpLogger>,
                                 RTC::Delete<AdcSitcpLogger>);
    }
};
