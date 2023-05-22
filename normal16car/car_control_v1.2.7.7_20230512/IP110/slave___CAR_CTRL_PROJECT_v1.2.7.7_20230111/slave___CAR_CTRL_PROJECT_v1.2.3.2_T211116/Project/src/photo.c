#include "photo.h"
#include "main.h"


sInput_Info  port_info[3];
sBD_SYS_MSG  sysmsg;

void vphoto_init_msg(void)
{
    sysmsg.photoreset = INVALUE;
    sysmsg.valuecnt = 0;
    sysmsg.interval = 0;
}

// reset photo
void vphoto_deal_with_reset_photo(void)
{
    u8 input = RESET_PHOTO_IN;

    if (port_info[0].input_state != input
        && port_info[0].input_confirm_times == 0)
    {
        port_info[0].input_middle_state = input;
    }
    if (port_info[0].input_middle_state == input
        && port_info[0].input_middle_state != port_info[0].input_state)
    {
        port_info[0].input_confirm_times++;
        if (port_info[0].input_confirm_times > PHOTO_RESET_FILTER)// 5ms
        {
            port_info[0].input_state = port_info[0].input_middle_state;
            port_info[0].input_confirm_times = 0;
            if (port_info[0].input_state == 1)
            {
                port_info[0].input_flag = INPUT_TRIG_HIGH;
                port_info[0].input_trig_mode = INPUT_TRIG_UP;
            }
            else
            {
                port_info[0].err_cnt = 0;
                port_info[0].input_flag = INPUT_TRIG_LOW;
            }
        }
    }
    else
    {
        port_info[0].input_middle_state = port_info[0].input_state;
        port_info[0].input_confirm_times = 0;
    }

    //  可以判断 光电报警 预留
    if (port_info[0].input_state == 1) {
        port_info[0].err_cnt++;
        if (port_info[0].err_cnt >= 5000) {
            port_info[0].err_cnt = 5000;
        }
    }

    if (port_info[0].input_trig_mode == INPUT_TRIG_UP) {
        port_info[0].input_trig_mode = INPUT_TRIG_NULL;

        if (sysmsg.photoreset == INVALUE) {
            sysmsg.photoreset = VALUE;
        }

        sysmsg.curposition = 0;

        sysmsg.resetlost = 0;
        //        sysmsg.interval = 0;
    }
}


// cnt photo
void vphoto_deal_with_cnt_photo(void)
{
    u8 input = CNT_PHOTO_IN;
    u8 buf[10] = { 0 };
    u8 sendlen = 0;

    sysmsg.interval++;
    if (sysmsg.valuecnt != 0) {
        sysmsg.valuecnt--;
        return;
    }

    if (port_info[1].input_state != input
        && port_info[1].input_confirm_times == 0)
    {
        port_info[1].input_middle_state = input;
    }
    if (port_info[1].input_middle_state == input
        && port_info[1].input_middle_state != port_info[1].input_state)
    {
        port_info[1].input_confirm_times++;
        if (port_info[1].input_confirm_times > PHOTO_CNT_FILTER)// 5ms
        {
            port_info[1].input_state = port_info[1].input_middle_state;
            port_info[1].input_confirm_times = 0;
            if (port_info[1].input_state == 1)
            {
                port_info[1].input_trig_mode = INPUT_TRIG_UP;
            }
            else
            {
                port_info[1].err_cnt = 0;
            }
        }
    }
    else
    {
        port_info[1].input_middle_state = port_info[1].input_state;
        port_info[1].input_confirm_times = 0;
    }

    //光电报警预留
    if (port_info[1].input_state == 1) {
        port_info[1].err_cnt++;
        if (port_info[1].err_cnt >= 5000) {
            port_info[1].err_cnt = 5000;
        }
    }

    if (port_info[1].input_trig_mode == INPUT_TRIG_UP) {
        port_info[1].input_trig_mode = INPUT_TRIG_NULL;

        if (sysmsg.valuecnt == 0) {
            if ((sysmsg.curposition <= 2000) && (sysmsg.photoreset == VALUE)) {
                sysmsg.curposition++;
                buf[0] = sysmsg.curposition & 0xFF;
                buf[1] = (sysmsg.curposition >> 8) & 0xFF;
                buf[2] = sysmsg.interval & 0xFF;
                buf[3] = (sysmsg.interval >> 8) & 0xFF;
                buf[4] = 0;
                buf[5] = 0;
                sysmsg.interval = 0;
                sendlen = 6;
                sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
                sysmsg.lostcount = 0;

                vcan_send_send_msg(buf, sendlen, CAN_FUNC_ID_SLAVER_PHOTO_CNT_TYPE, 1);
            }

            if ((sysmsg.curposition > max_guandian_count_u16) && (wcs2carSysEnData.sys_en == 1)) {
                sysmsg.resetlost++;
            }
        }
    }
}