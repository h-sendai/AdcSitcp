#include <arpa/inet.h>
#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int usage(void)
{
    fprintf(stderr, "Usage: docode data_file\n");
    return 0;
}

#define HEADER_SIZE 12
#define N_CH 16
FILE *fp;

int get_event_length(unsigned char *buf)
{
    unsigned int *int_p;
    unsigned int length;
    int_p = (unsigned int *)&buf[4];
    length = ntohl(*int_p);
    return length;
}

int get_module_num(unsigned char *buf)
{
    return buf[3];
}

int get_trigger_count(unsigned char *buf)
{
    unsigned int *int_p;
    unsigned int trigger_count;
    int_p = (unsigned int *)&buf[8];
    trigger_count = ntohl(*int_p);
    return trigger_count;
}

int read_header(unsigned char *buf)
{
    int n;
    n = fread(buf, 1, HEADER_SIZE, fp);
    if (n == 0) {
        if (feof(fp)) {
            exit(0);
        }
        else if (ferror(fp)) {
            err(1, "fread for header");
        }
    }
    if (n != HEADER_SIZE) {
        errx(1, "short read: %d (header)", n);
    }

    return 0;
}

int read_event_data(unsigned char *buf, int len)
{
    int n;
    n = fread(&buf[HEADER_SIZE], 1, len, fp);
    if (n == 0) {
        if (feof(fp)) {
            exit(0);
        }
        else if (ferror(fp)) {
            err(1, "fread for header");
        }
    }
    if (n != len) {
        errx(1, "short read: %d (event_data)", n);
    }
    return 0;
}

unsigned short get_data_at(unsigned char *buf, int ch, int sampling_num)
{
    unsigned short *short_p;
    unsigned short data;

    short_p = (unsigned short *)&buf[HEADER_SIZE + 2*sampling_num*N_CH + 2*ch];
    data = ntohs(*short_p);

    return data;
}
    
int main(int argc, char *argv[])
{
    int ch;
    unsigned char packet_buf[32*1024];
    int module_num;
    int event_length;
    int trigger_count;
    char *filename;
    int n_sampling;
    int data;
    int i;

    if (argc != 2) {
        usage();
        exit(1);
    }
    filename = argv[1];

    fp = fopen(filename, "r");
    if (fp == NULL) {
        err(1, "fopen");
    }

    for ( ; ; ) {
        read_header(packet_buf);
        event_length = get_event_length(packet_buf);
        module_num    = get_module_num(packet_buf);
        trigger_count = get_trigger_count(packet_buf);
        read_event_data(packet_buf, event_length);
        if (event_length % N_CH != 0) {
            errx(1, "event_length is not integer multiple of N_CH");
        }
        n_sampling = event_length / N_CH / 2; // 2: 2 bytes
        for (ch = 0; ch < N_CH; ch ++) {
            for (i = 0; i < n_sampling; i++) {
                data = get_data_at(packet_buf, ch, i);
                printf("module %d trigger %d ch %d window %d data %d\n",
                    module_num, trigger_count, ch, i, data);
            }
        }
    }
    return 0;
}
