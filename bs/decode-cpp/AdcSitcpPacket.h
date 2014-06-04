//
// Data Format
// 31                              0
// +--------------------------------+
// |  Type  |   ID  |   ID  |   ID  |
// +--------------------------------+
// |     Data  Length (data only)   |
// +--------------------------------+
// |          Trigger Count         |
// +--------------------------------+  <-- Window # 0
// |   ADC ch # 0   |  ADC ch # 1   |
// +--------------------------------+
// |   ADC ch # 2   |  ADC ch # 3   |
// +--------------------------------+
// |   ADC ch # 4   |  ADC ch # 5   |
//          :             :
// +--------------------------------+
// |   ADC ch # 14  |  ADC ch # 15  |
// +--------------------------------+  <-- Window # 1
// |   ADC ch # 0   |  ADC ch # 1   |
// +--------------------------------+
// |   ADC ch # 2   |  ADC ch # 3   |
// +--------------------------------+
// |   ADC ch # 4   |  ADC ch # 5   |
// +--------------------------------+
//          :             :
// |   ADC ch # 14  |  ADC ch # 15  |
// +--------------------------------+
//
// Type: 0x01
// ID: User defined values
// ADC data: 12bit network byte order

#ifndef _ADCSITCPPACKET
#define _ADCSITCPPACKET

#include <arpa/inet.h> // for ntohs(), ntohl()
#include <iostream>

class AdcSitcpPacket
{
public:
    AdcSitcpPacket();
    virtual ~AdcSitcpPacket();
    int set_buf(const unsigned char *buf, int buf_len);
    bool is_adc_sitcp_data_packet() const;
	//int  get_word_size();
    int  get_num_of_ch();
    int  get_data_length()   const;
	int  get_window_size()   const;
    int  get_trigger_count() const;
    int  get_buf_pos();
    int  get_buf_len();
	unsigned int get_data_at(int ch, int window) const;
	int  reset_buf();
	const static int HEADER_SIZE = 12;
    const static int N_CH        = 16;
    const static int TYPE_POS    = 0;
    const static int LENGTH_POS  = 4;
    const static int TRIGGER_POS = 8;
    const static int VALID_TYPE  = 0x01;

private:
    const unsigned char *m_buf;
    int m_buf_len;

};

#endif
