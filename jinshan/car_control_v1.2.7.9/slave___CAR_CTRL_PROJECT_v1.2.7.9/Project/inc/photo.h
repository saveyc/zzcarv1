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
    u16  curposition;                      //头车位置
    u16  interval;                         //光电之间的间隔
    u8   photoreset;                       //头车是否复位
    u16  valuecnt;                         //光电触发后 满足一定间隔 才能再次触发
    u8   cntlost;                          //板卡没有光电信号
    u16  lostcount;                        //板卡没有光电信号计时
    u16  resetlost;                        //复位信号丢失计次
}sBD_SYS_MSG;


#pragma pack () 


extern sInput_Info  port_info[];
extern sBD_SYS_MSG  sysmsg;

void vphoto_init_msg(void);
void vphoto_deal_with_reset_photo(void);
void vphoto_deal_with_cnt_photo(void);



#endif