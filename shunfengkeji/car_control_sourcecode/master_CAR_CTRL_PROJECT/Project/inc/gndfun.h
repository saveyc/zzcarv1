#ifndef _GNDFUN_H
#define _GNDFUN_H

// gnd发送位置信息
#define     RECV_MSG_GND2BD_CARPOS_TYPE                      0xA1A1
#define     REPLY_RECV_MSG_GND2BD_CARPOS_TYPE                0xA223

typedef struct {
    u16  photocnt;                   // 光电计数
    u16  intervel;                   // 光电间隔
    u16  error;                      // 光电错误
}sgndcnt;

extern sgndcnt  gndcnt;
extern u8 gndcntflag;
extern u8 gndfirstreset;

void vgndfun_init_net(void);
void vgndfun_add_gndfunmsg(u16 value);
void vgndfun_recvmessage_from_sever(u8* point, u16* len);
void vgndfun_send_message_to_sever(void);
void vgnd_deal_with_carpos_process(void);

#endif