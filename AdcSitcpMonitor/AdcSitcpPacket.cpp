#include "AdcSitcpPacket.h"

AdcSitcpPacket::AdcSitcpPacket(): m_buf(NULL), m_buf_len(-1)
{

}

AdcSitcpPacket::~AdcSitcpPacket()
{

}

int AdcSitcpPacket::set_buf(const unsigned char *buf, const int buf_len)
{
    m_buf = buf;
    m_buf_len = buf_len;

    return 0;
}

bool AdcSitcpPacket::is_adc_sitcp_data_packet() const
{
    unsigned char format = m_buf[TYPE_POS];
    if (format == AdcSitcpPacket::VALID_TYPE) {
        return true;
    }
    else {
        return false;
    }
}

int AdcSitcpPacket::get_data_length() const
{
    unsigned int *len;
    int rv;
    len = (unsigned int *)&m_buf[LENGTH_POS];
    rv = ntohl(*len);

    return rv;
}

int AdcSitcpPacket::get_trigger_count() const
{
    unsigned int *trig;
    int rv;
    trig = (unsigned int *)&m_buf[TRIGGER_POS];
    rv = ntohl(*trig);

    return rv;
}

unsigned int AdcSitcpPacket::get_data_at(int ch, int window) const
{
	unsigned short *data;

	int word_size = 2;
	int n_ch      = AdcSitcpPacket::N_CH;
	
	int offset = HEADER_SIZE + n_ch*word_size*window + ch*word_size;
	data = (unsigned short *)&m_buf[offset];
	unsigned short rv = ntohs(*data);
	rv = (rv & 0x0fff);
	return rv;
}

int AdcSitcpPacket::get_window_size() const
{
	int word_size = 2;
	int n_ch      = AdcSitcpPacket::N_CH;
	int data_length = get_data_length();

	int window_size = data_length / (word_size*n_ch);

	return window_size;
}

int AdcSitcpPacket::reset_buf()
{
	m_buf = NULL;
	m_buf_len = -1;
	
	return 0;
}
