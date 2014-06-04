// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef ADCSITCPREADER_H
#define ADCSITCPREADER_H

#include <sys/epoll.h>
#include <daqmw/Sock.h>
#include <err.h>

#include "DaqComponentBase.h"
#include "logUtil.h"

using namespace RTC;

struct module_info {
    DAQMW::Sock Sock;
    std::string ip_address;
    int         port;
    int         module_num;
    int         window_size;
    int         delay;
    unsigned char buf[8*1024]; // 8 kB buffer
};

class AdcSitcpReader
    : public DAQMW::DaqComponentBase
{
public:
    AdcSitcpReader(RTC::Manager* manager);
    ~AdcSitcpReader();

    // The initialize action (on CREATED->ALIVE transition)
    // former rtc_init_entry()
    virtual RTC::ReturnCode_t onInitialize();

    // The execution action that is invoked periodically
    // former rtc_active_do()
    virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

private:
    TimedOctetSeq          m_out_data;
    OutPort<TimedOctetSeq> m_OutPort;

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
    int set_data(unsigned char *buf, unsigned int data_byte_size);
    int write_OutPort();
    int set_window_size(std::string ip_address, int window_size);
    int set_delay(std::string ip_address, int delay);
    int set_packet_id(std::string ip_address, int module_num);
    int write_register_one_byte(std::string ip_address, int address, unsigned char data);
    int set_adc_registers(std::string ip_address);
    int get_data_byte_size(unsigned char *buf, int len);

    DAQMW::Sock* m_sock;               /// socket for data server

    static const  int WFDM_HEADER_BYTE_SIZE = 12;
    unsigned int  m_read_byte_size;
    int m_epfd;
    struct epoll_event *m_ev_ret;
    bool          m_set_registers;

    BufferStatus m_out_status;

    std::vector <struct module_info> m_module_list;

    bool m_debug;
};


extern "C"
{
    void AdcSitcpReaderInit(RTC::Manager* manager);
};

#endif // ADCSITCPREADER_H
