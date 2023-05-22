#ifndef _PHOTO_H
#define _PHOTO_H

#include "main.h"


#define  INPUT_TRIG_NULL    0
#define  INPUT_TRIG_UP      1
#define  INPUT_TRIG_DOWN    2

#define  INPUT_TRIG_LOW     0
#define  INPUT_TRIG_HIGH    1

#define  RESET_PHOTO_IN   INPUT3_STATUE
#define  CNT_PHOTO_IN     INPUT2_STATUE


#define  PHOTO_RESET_FILTER    3
#define  PHOTO_CNT_FILTER      3

#define  PHOTO_INTERVAL_MIN    100



#pragma pack (1) 

typedef struct {
    u8  input_state;
    u8  input_middle_state;
    u16 input_confirm_times;
    u8  input_trig_mode; //null,up,down
    u8  input_flag;
    u16 err_cnt;
}sInput_Info;

typedef  struct {
    u16  curposition;                      //ͷ��λ��
    u16  interval;                         //���֮��ļ��
    u8   photoreset;                       //ͷ���Ƿ�λ
    u16  valuecnt;                         //��紥���� ����һ����� �����ٴδ���
    u8   cntlost;                          //�忨û�й���ź�
    u16  lostcount;                        //�忨û�й���źż�ʱ
    u16  resetlost;                        //��λ�źŶ�ʧ�ƴ�
}sBD_SYS_MSG;


#pragma pack () 


extern sInput_Info  port_info[];
extern sBD_SYS_MSG  sysmsg;

void vphoto_init_msg(void);
void vphoto_deal_with_reset_photo(void);
void vphoto_deal_with_cnt_photo(void);



#endif