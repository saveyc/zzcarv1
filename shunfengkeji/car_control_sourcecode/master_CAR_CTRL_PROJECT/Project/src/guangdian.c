#include "stm32f10x.h"
#include "stm32f107.h"
#include "main.h"

uint16_t  INPUT2_count = 0;
uint16_t  pre_INPUT2_count;
uint8_t   guangdian_error = 0;
u16 INPUT2_count_error = 0;
u8  moto_statue_comm_en = 0;

extern u16 s_LED_Interval;

void Photoeletricity_Process(u8 extIntOpt)
{
    u8 i;
    sCar2Wcs_headcar_positon node;

    if (extIntOpt == INT_OPT_RST)//复位信号
    {
        if (sysmsg.resetcnt == 0) {

            if (wcs2carSysEnData.sys_en == 0) {
                sysmsg.resetlost = 0;
                INPUT2_count = 0;
                sysmsg.resetcnt = PHOTO_RESET_MIN;
            }

            if (wcs2carSysEnData.sys_en == 1)
            {
                if (INPUT2_count >= max_guandian_count_u16) {
                    sysmsg.resetlost = 0;
                    INPUT2_count = 0;
                    sysmsg.resetcnt = PHOTO_RESET_MIN;
                }
            }

            if (sysmsg.photoreset == INVALUE) {
                sysmsg.photoreset = VALUE;
                sysmsg.slaverrst = VALUE;
                sysmsg.pcslaverrst = VALUE;
            }
        }
    }

    if (extIntOpt == INT_OPT_INC)//计数信号
    {
        if (sysmsg.valuecnt == 0) {
            if (INPUT2_count < max_guandian_count_u16)
            {
                sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
                INPUT2_count++;
                headcarpositoon.position = INPUT2_count;
                node.position = INPUT2_count;
                node.interval = headcarpositoon.interval;
                vphoto_add_to_slaver_position_queue(node);
                headcarpositoon.interval = 0;
//                AddSendMsgToQueue(SEND_MSG_CAR2WCS_CARPOSITION_TYPE);
                can_bus_send_photo_electricity(0, 0, INPUT2_count);
                sysmsg.lostcount = 0;
                sysmsg.resetlost = 0;

                if (wcs2carSysEnData.sys_en == 1)
                {
                    xialiao_process();
                }

                if (INPUT2_count == 2)
                {
                    moto_statue_comm_en = 1;

                    if (guangdian_error == 1)
                    {
                        for (i = 0; i < MOTO_MAX_COUNT; i++)
                        {
                            moto_para[i].statue |= 0x10;
                        }
                    }
                    else
                    {
                        for (i = 0; i < MOTO_MAX_COUNT; i++)
                        {
                            moto_para[i].statue &= 0xef;
                        }
                    }
                    for (i = 0; i < MOTO_MAX_COUNT; i++)
                    {
                        sys_moto_statue[XIAOCHE_START_NUM - 1 + i] = moto_para[i].statue;
                    }
                    statue_car_num = XIAOCHE_START_NUM;
                    statue_car_count = MOTO_MAX_COUNT;
                    statue_guangdian_num = INPUT2_count_error | XIAOCHE_GROUP_NUM;
                }
            }
            else
            {
                if (wcs2carSysEnData.sys_en == 1)
                {
                    if (s_LED_Interval == 0)
                        s_LED_Interval = 400;

                    guangdian_error = 1;
                    INPUT2_count_error |= 0x8000;//多光电
                    moto_statue_comm_en = 1;
                    sysmsg.resetlost++;
                    if (sysmsg.resetlost > 5) {
                        sysmsg.cntlost = 2;
                        sysmsg.resetlost = 0;
                    }
                }
                
            }
            sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
        }

    }
}
////sys_tick中1ms调用一次
//void Ext_Opt_Process(void)
//{
//    if(opt_inc_flag)//计数光电去抖
//    {
//        if(INPUT2_STATUE == 0)
//        {
//            opt_inc_flag = 0;
//            opt_inc_cnt = 0;
//        }
//        else
//        {
//            if(opt_inc_cnt > 2)
//            {
//                opt_inc_flag = 0;
//                opt_inc_cnt = 0;
//                
//                Photoeletricity_Process(INT_OPT_INC);
//            }
//            else
//            {
//                opt_inc_cnt++;
//            }
//        }
//    }
//    if(opt_rst_flag)//复位光电去抖
//    {
//        if(INPUT3_STATUE == 0)
//        {
//            opt_rst_flag = 0;
//            opt_rst_cnt = 0;
//        }
//        else
//        {
//            if(opt_rst_cnt > 2)
//            {
//                opt_rst_flag = 0;
//                opt_rst_cnt = 0;
//                
//                Photoeletricity_Process(INT_OPT_RST);
//            }
//            else
//            {
//                opt_rst_cnt++;
//            }
//        }
//    }
//}








