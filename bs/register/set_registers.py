#!/usr/bin/env python

import time
import SitcpRbcp

def get_board_version(ip_address):
    print ip_address
    rbcp = SitcpRbcp.SitcpRbcp()
    reg_value = rbcp.read_register_f(ip_address, 0x4, '>B')
    if reg_value == 0x02:
        print 'V2 board'
        return 2 
    else:
        print 'V1 board'
        return 1

def main():
    rbcp = SitcpRbcp.SitcpRbcp()
    #rbcp.set_verify_mode()
    rbcp.set_timeout(1.0)
    ip_address_list = [
        '192.168.10.16',
#        '192.168.10.17'
    ]
    
    for ip_address in ip_address_list:
        #board_version = get_board_version(ip_address)
        board_version = 1

        # reset ADC (soft reset)
        rbcp.write_register_f(ip_address, 0x10,  '>B', 0x0)
        rbcp.write_register_f(ip_address, 0x100, '>B', 0x3c)
        rbcp.write_register_f(ip_address, 0x10,  '>B', 0x1)
        rbcp.write_register_f(ip_address, 0x100, '>B', 0x3c)

        # internal power down mode (do reset)
        rbcp.write_register_f(ip_address, 0x10,  '>B', 0x0)
        rbcp.write_register_f(ip_address, 0x108, '>B', 0x3)
        rbcp.write_register_f(ip_address, 0x10,  '>B', 0x1)
        rbcp.write_register_f(ip_address, 0x108, '>B', 0x3)
        time.sleep(0.1)

        # internal power down mode (do release reset)
        rbcp.write_register_f(ip_address, 0x10,  '>B', 0x0)
        rbcp.write_register_f(ip_address, 0x108, '>B', 0x0)
        rbcp.write_register_f(ip_address, 0x10,  '>B', 0x1)
        rbcp.write_register_f(ip_address, 0x108, '>B', 0x0)

        if board_version == 1:
            # serial outout number of bits (set 12 bits)
            rbcp.write_register_f(ip_address, 0x10,  '>B', 0x0)
            rbcp.write_register_f(ip_address, 0x121, '>B', 0x3)
            rbcp.write_register_f(ip_address, 0x1ff, '>B', 0x1)
            rbcp.write_register_f(ip_address, 0x10,  '>B', 0x1)
            rbcp.write_register_f(ip_address, 0x121, '>B', 0x3)
            rbcp.write_register_f(ip_address, 0x1ff, '>B', 0x1)
            
        # Output format: offset binary
        rbcp.write_register_f(ip_address, 0x10, '>B', 0x0)
        rbcp.write_register_f(ip_address, 0x114, '>B', 0x0)
        rbcp.write_register_f(ip_address, 0x10, '>B', 0x1)
        rbcp.write_register_f(ip_address, 0x114, '>B', 0x0)

        # fpga adc interface reset
        rbcp.write_register_f(ip_address, 0x5, '>B', 0x1)
        rbcp.write_register_f(ip_address, 0x5, '>B', 0x0)
    
if __name__ == '__main__':
    main()
