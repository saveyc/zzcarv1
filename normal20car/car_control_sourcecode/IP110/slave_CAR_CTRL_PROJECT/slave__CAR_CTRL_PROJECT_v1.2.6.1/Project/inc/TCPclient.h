#ifndef __TCPCLIENT_H__
#define __TCPCLIENT_H__

#define	TCP_CLIENT_MAX_NUM	2

struct tcp_client_table
{
    u8  tcp_recev_buf[TCP_RECEV_BUFF_SIZE];
    u8  tcp_send_buf[TCP_SEND_BUFF_SIZE];
    u8  tcp_send_en;
    u16 tcp_recev_len;
    u16 tcp_send_len;
    u8  tcp_client_statue;
    u16 tcp_client_Delay_reg;
#ifndef     USE_UDP
    struct tcp_pcb *echoclient_pcb;
#else
    struct udp_pcb *echoclient_pcb;
#endif
    u8  dest_ip[4];
    u16 dest_port;
    u8  connect_is_ok;
    u8  connect_dely;
};
extern u16 WCS_ACK_timer;
extern u8 TCP_send_dely;
extern struct tcp_client_table	tcp_client_list[];
#ifndef     USE_UDP
void tcp_echoclient_connect(u8 tcp_offset);
void tcp_client_process(void);
#else
extern void udp_send_process(u8 udp_offset);
extern void udp_echoclient_connect(u8 udp_offset);
extern void udp_client_process(void);
#endif
extern void WCS_TCP_reply_process(u8 *p_data, u16 len);

#endif