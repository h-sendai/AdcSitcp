#include <arpa/inet.h>
#include <err.h>
#include <cstdio>
#include <iostream>

#include <daqmw/Sock.h>
#include "AdcSitcpReader.h"
#include "SitcpRbcp.h"

int AdcSitcpReader::write_register_one_byte(std::string ip_address, int address, unsigned char data)
{
    SitcpRbcp bcp;

    unsigned char data_buf[1];

    data_buf[0] = data;
    if (bcp.write_registers(ip_address, address, 1, data_buf) < 0) {
        warnx("write_register_one_byte fail");
        return -1;
    }
    
    return 0;
}

int get_board_version(std::string ip_address)
{
    SitcpRbcp bcp;
    unsigned char reg_value[1];
    unsigned char board_version;

    if (bcp.read_registers(ip_address, 0x4, 1, reg_value) < 0) {
        return -1;
    }

    board_version = reg_value[0];
    board_version = (board_version & 0xF0);
    board_version = (board_version >> 4);

    if (board_version == 2) {
        return 2; // Version 2 board
    }
    else {
        return 1; // Version 1 board
    }
    
    return 0; // make compiler happy
}

int AdcSitcpReader::set_adc_registers(std::string ip_address)
{
    int board_version = get_board_version(ip_address);
    if (board_version < 0) {
       return -1;
    }

    // reset ADC (soft reset)
    write_register_one_byte(ip_address, 0x10,  0x0);
    write_register_one_byte(ip_address, 0x100, 0x3c);
    write_register_one_byte(ip_address, 0x10,  0x1);
    write_register_one_byte(ip_address, 0x100, 0x3c);
    
    // internal power down mode (do reset)
    write_register_one_byte(ip_address, 0x10,  0x0);
    write_register_one_byte(ip_address, 0x108, 0x3);
    write_register_one_byte(ip_address, 0x10,  0x1);
    write_register_one_byte(ip_address, 0x108, 0x3);
    usleep(100000);

    // internal power down mode (do release reset)
    write_register_one_byte(ip_address, 0x10,  0x0);
    write_register_one_byte(ip_address, 0x108, 0x0);
    write_register_one_byte(ip_address, 0x10,  0x1);
    write_register_one_byte(ip_address, 0x108, 0x0);
    
    if (board_version == 1) {
       write_register_one_byte(ip_address, 0x10,  0x0);
       write_register_one_byte(ip_address, 0x121, 0x3);
       write_register_one_byte(ip_address, 0x1ff, 0x1);
       write_register_one_byte(ip_address, 0x10,  0x1);
       write_register_one_byte(ip_address, 0x121, 0x3);
       write_register_one_byte(ip_address, 0x1ff, 0x1);
    }

    // Ouput format: offset binary
    write_register_one_byte(ip_address, 0x10,  0x0);
    write_register_one_byte(ip_address, 0x114, 0x0);
    write_register_one_byte(ip_address, 0x10,  0x1);
    write_register_one_byte(ip_address, 0x114, 0x0);

    // FPGA ADC interface reset
    write_register_one_byte(ip_address, 0x5, 0x1);
    write_register_one_byte(ip_address, 0x5, 0x0);

    return 0;
}

int AdcSitcpReader::set_window_size(std::string ip_address, int window_size)
{
    SitcpRbcp bcp;

    int length  = 1;
    int address = 0x6;
    unsigned char data_buf[length];

    // Register value for Window size
    // Register value:  0 means 1
    // Register value:  1 means 2
    // Register value: 29 means 30
    // :
    // Register value: 127: means 128 (max value)
    data_buf[0] = window_size - 1;

    if (bcp.write_registers(ip_address, address, length, data_buf) < 0) {
        warnx("registers for window_size setting fail");
        return -1;
    }

    return 0;
}

int AdcSitcpReader::set_delay(std::string ip_address, int delay)
{
    SitcpRbcp bcp;

    int length  = 1;
    int address = 0x7;
    unsigned char data_buf[length];

    // Register value for delay
    // 0 - 127
    // Register value: 127: means 128 (max value)
    data_buf[0] = delay;

    if (bcp.write_registers(ip_address, address, length, data_buf) < 0) {
        warnx("registers for delay setting fail");
        return -1;
    }

    return 0;
}

int AdcSitcpReader::set_packet_id(std::string ip_address, int module_num)
{
    SitcpRbcp bcp;

    int length  = 3;
    int address = 0x0a;
    unsigned char data_buf[length];

    data_buf[0] = 0x00;
    data_buf[1] = 0x00;
    data_buf[2] = module_num;

    if (bcp.write_registers(ip_address, address, length, data_buf) < 0) {
        warnx("registers for packet_id setting fail");
        return -1;
    }

    return 0;
}
