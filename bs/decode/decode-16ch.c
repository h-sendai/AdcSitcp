#include <err.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HEADER_SIZE (4*3)
#define DATA_SIZE (1024*1024)
#define N_CHANNEL 16

/* Data Format
 * +-------------------+
 * |    |    |    |    |  buf[0]: 0x01, buf[1] - buf[3]: packet id
 * +-------------------+
 * |     Data Length   |  Data Length (only the data part in bytes size)
 * +-------------------+
 * |   trigger Counter |
 * +-------------------+
 * |  CH 0   |  CH 1   |
 * +-------------------+
 * |         |         |
 * +-------------------+
 * |         |         |
 * +-------------------+
 * |         |  CH 15  |
 * +-------------------+
 * |  CH 0   |  CH 1   |
 * +-------------------+
 * |         |         |
 * +-------------------+
 * |         |         |
 * +-------------------+
 * |         |  CH 15  |
 * +-------------------+
 *   Data part follows
 */

int usage()
{
    fprintf(stderr, "Usage: prog filename\n");
    fprintf(stderr, "Print module_number channel_number trigger_counter data\n");
    return 0;
}

int main(int argc, char *argv[])
{
    FILE *fp;
    unsigned char header_buf[HEADER_SIZE];
    unsigned char data_buf[DATA_SIZE];
    int length;
    int trigger;
    int data_set;
    short data;
    int ch;
    int i;
    int c;
    int print_trigger_num = 0;
    int n;
    unsigned int *int_p;
    unsigned short *short_p;
    int channel = -1;
    int module_num = -1;
    int mod_num;
    int print_header_in_hex = 1;

    while ( (c = getopt(argc, argv, "c:m:t")) != -1) {
        switch (c) {
            case 'c':
                channel = strtol(optarg, NULL, 0);
                break;
            case 'm':
                module_num = strtol(optarg, NULL, 0);
                break;
            case 't':
                print_trigger_num = 1;
                break;
            default:
                break;
        }
    }

    argc -= optind;
    argv += optind;
        
    if (argc != 1) {
        usage();
        exit(1);
    }

    if ( (fp = fopen(argv[0], "r")) == NULL) {
        err(1, "fopen");
    }

    for ( ; ; ) {
        n = fread(header_buf, 1, HEADER_SIZE, fp);
        if (n == 0) {
            if (feof(fp)) {
                break;
            }
            else if (ferror(fp)) {
                err(1, "fread error");
            }
        }
        if (n != HEADER_SIZE) {
            errx(1, "short read: %d bytes (should be %d bytes)", n, HEADER_SIZE);
        }
        int_p = (unsigned int *)&header_buf[4];
        length = ntohl(*int_p);
        int_p = (unsigned int *)&header_buf[8];
        trigger = ntohl(*int_p);
        mod_num = header_buf[3];
        if (print_header_in_hex) {
            printf("#### ");
            for (i = 0; i < 11; i++) {
                printf("%02x ", header_buf[i]);
                if ((i + 1) % 4 == 0) {
                    printf("| ");
                }
            }
            printf("%02x\n", header_buf[11]);
        }

        n = fread(data_buf, 1, length, fp);
        if (n == 0) {
            if (feof(fp)) {
                break;
            }
            else if (ferror(fp)) {
                err(1, "fread error");
            }
        }
        if (n != length) {
            errx(1, "short read: %d bytes (should be %d bytes)", n, length);
        }
        
        data_set = length / 2 / N_CHANNEL; /* 16 ch */

        for (i = 0; i < data_set; i++) {
            for (ch = 0; ch < N_CHANNEL; ch ++) {
                int pos = 2*N_CHANNEL*i + 2*ch;
                short_p = (unsigned short *)&data_buf[pos];
                data = ntohs(*short_p);
                printf("%d %d %d %d\n", mod_num, ch, trigger, data);
            }
        }
    }

    return 0;
}
