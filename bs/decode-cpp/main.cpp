#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdio>
#include <fstream>
#include <iostream>

#include "AdcSitcpPacket.h"

int usage()
{
    fprintf(stderr, "Usage: ./main filename\n");

    return 0;
}

//int decode(unsigned char *buf, int len)
int decode(const unsigned char *buf, int len)
{
    AdcSitcpPacket asp;
    asp.set_buf(buf, len);
    if (! asp.is_adc_sitcp_data_packet()) {
        errx(1, "not a Adc Sitcp packet");
    }

    // int data_len      = asp.get_data_length();
    int trigger_count = asp.get_trigger_count();
    int window_size   = asp.get_window_size();

    for (int ch = 0; ch < AdcSitcpPacket::N_CH; ++ch) {
        for (int w = 0; w < window_size; ++w) {
            int data = asp.get_data_at(ch, w);
            printf("trig: %d | ch: %d | window: %d | data: %d\n",
                trigger_count, ch, w, data);
        }
    }
    asp.reset_buf();

    return 0;
}

int main(int argc, char *argv[])
{
    AdcSitcpPacket asp;

    FILE *fp;
    unsigned char buf[32*1024];
    int n;

    if (argc != 2) {
        usage();
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        err(1, "%s", argv[1]);
    }
    
    for ( ; ; ) {
        n = fread(buf, 1, AdcSitcpPacket::HEADER_SIZE, fp);
        if (n == 0) {
            if (feof(fp)) {
                break;
            }
            else if (ferror(fp)) {
                err(1, "fread");
            }
        }
        if (n != AdcSitcpPacket::HEADER_SIZE) {
            errx(1, "partial read %d bytes (should be %d bytes)",
                n, AdcSitcpPacket::HEADER_SIZE);
        }
        asp.set_buf(buf, AdcSitcpPacket::HEADER_SIZE);
        int data_len = asp.get_data_length();
        asp.reset_buf();

        n = fread(&buf[AdcSitcpPacket::HEADER_SIZE], 1, data_len, fp);
        if (n == 0) {
            if (feof(fp)) {
                break;
            }
            else if (ferror(fp)) {
                err(1, "fread");
            }
        }
        if (n != data_len) {
            errx(1, "partial read %d bytes (should be %d bytes)",
                n, data_len);
        }
        decode(buf, AdcSitcpPacket::HEADER_SIZE + data_len);
    }
    return 0;
}
