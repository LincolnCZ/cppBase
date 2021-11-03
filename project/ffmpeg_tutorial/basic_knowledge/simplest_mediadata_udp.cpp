#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

//#pragma comment(lib, "ws2_32.lib")

#pragma pack(1)

/**
 * [memo] FFmpeg stream Command:
 *
 * 下面的命令可以推流UDP封装的MPEG-TS
 * ffmpeg -re -i sintel.ts -f mpegts udp://127.0.0.1:8880
 *
 * 下面的命令可以推流首先经过RTP封装，然后经过UDP封装的MPEG-TS
 * ffmpeg -re -i sintel.ts -f rtp_mpegts udp://127.0.0.1:8880
 */

typedef struct RTP_FIXED_HEADER {
    /* byte 0 */
    unsigned char csrc_len: 4;       /* expect 0 */
    unsigned char extension: 1;      /* expect 1 */
    unsigned char padding: 1;        /* expect 0 */
    unsigned char version: 2;        /* expect 2 */
    /* byte 1 */
    unsigned char payload: 7;
    unsigned char marker: 1;        /* expect 1 */
    /* bytes 2, 3 */
    unsigned short seq_no;
    /* bytes 4-7 */
    unsigned long timestamp;
    /* bytes 8-11 */
    unsigned long ssrc;            /* stream number is used here. */
} RTP_FIXED_HEADER;

typedef struct MPEGTS_FIXED_HEADER {
    unsigned sync_byte: 8;
    unsigned transport_error_indicator: 1;
    unsigned payload_unit_start_indicator: 1;
    unsigned transport_priority: 1;
    unsigned PID: 13;
    unsigned scrambling_control: 2;
    unsigned adaptation_field_exist: 2;
    unsigned continuity_counter: 4;
} MPEGTS_FIXED_HEADER;


int simplest_udp_parser(int port) {
    int cnt = 0;

    //FILE *myout=fopen("output_log.txt","wb+");
    FILE *myout = stdout;

    FILE *fp1 = fopen("output_dump.ts", "wb+");

    // server
    int server_fd, ret;
    struct sockaddr_in ser_addr;

    server_fd = socket(AF_INET, SOCK_DGRAM, 0); //AF_INET:IPV4;SOCK_DGRAM:UDP
    if (server_fd < 0) {
        printf("create socket fail!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址，需要进行网络序转换，INADDR_ANY：本地地址
    ser_addr.sin_port = htons(port);  //端口号，需要网络序转换

    ret = bind(server_fd, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
    if (ret < 0) {
        printf("socket bind fail!\n");
        return -1;
    }

    // client
    sockaddr_in remoteAddr;
    int nAddrLen = sizeof(remoteAddr);

    //How to parse?
    int parse_rtp = 1;
    int parse_mpegts = 1;

    printf("Listening on port %d\n", port);

    char recvData[10000];
    while (1) {

        //recvfrom是拥塞函数，没有数据就一直拥塞
        int pktsize = recvfrom(server_fd, recvData, 10000, 0, (struct sockaddr *) &remoteAddr,
                               reinterpret_cast<socklen_t *>(&nAddrLen));
        if (pktsize > 0) {
            //printf("Addr:%s\r\n",inet_ntoa(remoteAddr.sin_addr));
            //printf("packet size:%d\r\n",pktsize);
            //Parse RTP
            //
            if (parse_rtp != 0) {
                char payload_str[10] = {0};
                RTP_FIXED_HEADER rtp_header;
                int rtp_header_size = sizeof(RTP_FIXED_HEADER);
                //RTP Header
                memcpy((void *) &rtp_header, recvData, rtp_header_size);

                //RFC3351
                char payload = rtp_header.payload;
                switch (payload) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                        sprintf(payload_str, "Audio");
                        break;
                    case 31:
                        sprintf(payload_str, "H.261");
                        break;
                    case 32:
                        sprintf(payload_str, "MPV");
                        break;
                    case 33:
                        sprintf(payload_str, "MP2T");
                        break;
                    case 34:
                        sprintf(payload_str, "H.263");
                        break;
                    case 96:
                        sprintf(payload_str, "H.264");
                        break;
                    default:
                        sprintf(payload_str, "other");
                        break;
                }

                unsigned int timestamp = ntohl(rtp_header.timestamp);
                unsigned int seq_no = ntohs(rtp_header.seq_no);

                fprintf(myout, "[RTP Pkt] %5d| %5s| %10u| %5d| %5d|\n", cnt, payload_str, timestamp, seq_no, pktsize);

                //RTP Data
                char *rtp_data = recvData + rtp_header_size;
                int rtp_data_size = pktsize - rtp_header_size;
                fwrite(rtp_data, rtp_data_size, 1, fp1);

                //Parse MPEGTS
                if (parse_mpegts != 0 && payload == 33) {
                    MPEGTS_FIXED_HEADER mpegts_header;
                    for (int i = 0; i < rtp_data_size; i = i + 188) {
                        if (rtp_data[i] != 0x47)
                            break;
                        //MPEGTS Header
                        //memcpy((void *)&mpegts_header,rtp_data+i,sizeof(MPEGTS_FIXED_HEADER));
                        fprintf(myout, "   [MPEGTS Pkt]\n");
                    }
                }

            } else {
                fprintf(myout, "[UDP Pkt] %5d| %5d|\n", cnt, pktsize);
                fwrite(recvData, pktsize, 1, fp1);
            }

            cnt++;
        }
    }

    close(server_fd);
    fclose(fp1);

    return 0;
}
