#include "stm32f10x.h"
#include "stm32f107.h"
#include "main.h"
uint32_t	INPUT2_L_timer, INPUT2_H_timer;
uint32_t    pre_INPUT2_L_timer, pre_INPUT2_H_timer;
uint32_t	INPUT2_timer = 0;
uint32_t	pre_INPUT2_timer = 0;
uint16_t	INPUT2_timer_beilv = 0;
uint16_t	INPUT2_count = 1;
uint16_t 	pre_INPUT2_count, cur_INPUT2_count;
uint8_t		per_INPUT2_statue = 0;
uint8_t     start_count_en = 0;
uint8_t     guangdian_error = 0;
u32  zhuxian_speed = 0;
uint16_t  err_count = 0;
uint16_t  gd_count = 0;
u16 INPUT2_count_error = 0;
u8 moto_statue_comm_en = 0;

/*********************************************

**********************************************/
#ifdef _UDP_DEBUG_
extern u8 debug_info[500];
extern void  DEBUG_process_udp(u8 *p_data, u16 len);
#endif

u16 s_guangdian_recover_timer_16 = 0;
extern u32 s_speed_zhuxian_u32;
extern u16 led_on_delay;
extern u8 slave_send_debug_flag;

u32 s_LastPhotoCount = 0xffffffff;

void recv_photoeletricity()
{
    u16 tmp = 0;
    u32 yushu_u32 = 0;

    //s_guangdian_recover_timer_16 = s_speed_zhuxian_u32 + 40;

    led_on_delay = 15;
    LED_ON;

    guangdian_error = Debug_recv_buff[0] >> 1;
    tmp = *((u16 *)(&Debug_recv_buff[1]));

    if(tmp >= XIAOCHE_START_NUM)//转换成从车自己的光电计数
    {
        INPUT2_count = tmp - XIAOCHE_START_NUM + 1;
    }
    else
    {
        INPUT2_count = max_guandian_count_u16 - (XIAOCHE_START_NUM - tmp) + 1;
    }

    if(s_LastPhotoCount != 0xffffffff)
    {
        if(INPUT2_count >= s_LastPhotoCount)
        {
            yushu_u32 = INPUT2_count - s_LastPhotoCount;
        }
        else
        {
            yushu_u32 = max_guandian_count_u16 - s_LastPhotoCount + INPUT2_count;
        }
//        if((yushu_u32 >= 2) && (wcs2carSysEnData.sys_en == 1))
//        {
//            INPUT2_count_error |= 0x800;
//        }
        if(wcs2carSysEnData.sys_en == 1)
        {
            if(yushu_u32 == 0)//重复的光电号
            {
                return;
            }
            else if(yushu_u32 == 2)//丢失一次光电
            {
                INPUT2_count_error |= 0x1000;
            }
            else if(yushu_u32 > 2)//连续丢失两次光电
            {
                INPUT2_count_error |= 0x800;
            }
        }
    }
    s_LastPhotoCount = INPUT2_count;

    if(guangdian_error &&  (wcs2carSysEnData.sys_en == 1))
    {
        slave_send_debug_flag = 5;
    }

    if((wcs2carSysEnData.sys_en == 1))
    {
        xialiao_process();
    }

    if( (INPUT2_count == 2) && (wcs2carSysEnData.sys_en == 1) )
    {
        slave_send_debug_flag = 5;
    }
}