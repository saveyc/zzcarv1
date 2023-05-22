#include "photo.h"

sInput_Info  port_info[4];

#define    PHOTOCNTMAX         20
sphotobdcnt        photobdcnt[PHOTOCNTMAX];
sphotobdcntqueue   photobdcntqueue;

#define    POSITIONCNTMAX      20
sCar2Wcs_headcar_positon     positionnode[POSITIONCNTMAX];
sCar2Wcs_headcar_queue       positionqueue;

u8   photo_bd_cnt_buff[10];

sBD_SYS_MSG                  sysmsg;
sBD_SYS_MSG                  phototrig;

sphotobdcnt  slaverbdcnt;
u8 slavercntflag = INVALUE;

u16 upload_photoone_cnt = 0;
u16 upload_phototwo_cnt = 0;
u16 upload_photolast_cnt = 0;

u16 photolast_num = 0xFFFF;
u16 photoone_num = 0xFFFF;
u16 phototwo_num = 0xFFFF;

u8 slavefirstreset = INVALUE;

void vphoto_init_slaver_position_queue(void)
{
    sCar2Wcs_headcar_queue* q = NULL;
    q = &positionqueue;

    q->queue = positionnode;
    q->maxsize = POSITIONCNTMAX;
    q->front = q->rear = 0;

}

void vphoto_add_to_slaver_position_queue(sCar2Wcs_headcar_positon x)
{
    sCar2Wcs_headcar_queue* q = NULL;
    q = &positionqueue;

    if (((q->rear + 1) % q->maxsize) == (q->front)) {
        return;
    }

    q->rear = (q->rear + 1) % (q->maxsize);

    q->queue[q->rear] = x;
}

sCar2Wcs_headcar_positon* vphoto_get_from_positionqueue(void)
{
    sCar2Wcs_headcar_queue* q = NULL;
    q = &positionqueue;

    if ((q->front) == (q->rear)) {
        return NULL;
    }

    q->front = (q->front + 1) % (q->maxsize);

    return (sCar2Wcs_headcar_positon*)(&(q->queue[q->front]));
}

void vphoto_upload_positionqueue(void)
{
    sCar2Wcs_headcar_queue* q = NULL;
    q = &positionqueue;

    if ((q->front) == (q->rear)) {
        return;
    }

    AddSendMsgToQueue(SEND_MSG_CAR2WCS_CARPOSITION_TYPE);
}


void vphoto_init_slaver_bdcnt_queue(void)
{
    sphotobdcntqueue* q = NULL;
    q = &photobdcntqueue;

    q->queue = photobdcnt;
    q->maxsize = PHOTOCNTMAX;
    q->front = q->rear = 0;

    sysmsg.curposition = 0;
    sysmsg.interval = 0;
    sysmsg.photoreset = INVALUE;
    sysmsg.valuecnt = 0;
    sysmsg.resetcnt = 0;
    sysmsg.slaverrst = 0;
    sysmsg.lostcount = 0;
    sysmsg.resetlost = 0;
    sysmsg.cntlost = 0;

    phototrig.curposition = 0;
    phototrig.interval = 0;
    phototrig.photoreset = INVALUE;
    phototrig.valuecnt = 0;
    phototrig.resetcnt = 0;


}

void vphoto_add_to_slaver_bd_queue(sphotobdcnt x)
{
    sphotobdcntqueue* q = NULL;
    q = &photobdcntqueue;

    if (((q->rear + 1) % q->maxsize) == (q->front)) {
        return;
    }

    q->rear = (q->rear + 1) % (q->maxsize);

    q->queue[q->rear] = x;
}

sphotobdcnt* vphoto_get_from_slaverbdqueue(void)
{
    sphotobdcntqueue* q = NULL;
    q = &photobdcntqueue;

    if ((q->front) == (q->rear)) {
        return NULL;
    }

    q->front = (q->front + 1) % (q->maxsize);

    return (sphotobdcnt*)(&(q->queue[q->front]));
}

void vphoto_upload_slaverbdqueue(void)
{
    sphotobdcntqueue* q = NULL;
    q = &photobdcntqueue;

    if ((q->front) == (q->rear)) {
        return;
    }

    AddSendMsgToQueue(SEND_MSG_BD2PC_SLAVER_PHOTO_CNT);


}


void vphoto_recv_slaver_photo_cmd(u8 bdinde, u16 cnt)
{
    u16 tempcnt = 0;
    u16 i = 0;
    sCar2Wcs_headcar_positon node;

    tempcnt = (bdinde - 1) * 16 + cnt;

    if (max_guandian_count_u16 == 0) {
        return;
    }
    tempcnt = (tempcnt + max_guandian_count_u16) % max_guandian_count_u16;
    if (tempcnt == 0) {
        tempcnt = max_guandian_count_u16;
    }

    if (tempcnt == 1) {
        if (sysmsg.resetcnt == 0) {

            if ((wcs2carSysEnData.sys_en == 1) && (sysmsg.photoreset == VALUE))
            {
                if (INPUT2_count < max_guandian_count_u16)
                {
                    //                    if (s_LED_Interval == 0)
                    //                        s_LED_Interval = 2000;

                    guangdian_error = 1;
                    INPUT2_count_error |= 0x4000;//少光电
                    moto_statue_comm_en = 1;
                }
            }

            sysmsg.resetcnt = PHOTO_RESET_MIN;
            sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
            INPUT2_count = 1;
            headcarpositoon.position = INPUT2_count;
            node.position = INPUT2_count;
            node.interval = headcarpositoon.interval;
            vphoto_add_to_slaver_position_queue(node);
            headcarpositoon.interval = 0;
//            AddSendMsgToQueue(SEND_MSG_CAR2WCS_CARPOSITION_TYPE);
            can_bus_send_photo_electricity(0, 0, INPUT2_count);

            if (wcs2carSysEnData.sys_en == 1)
            {
                xialiao_process();
            }

            if (sysmsg.photoreset == INVALUE) {
                sysmsg.photoreset = VALUE;
                sysmsg.slaverrst = VALUE;
                sysmsg.pcslaverrst = VALUE;
            }
        }
        else {
            if (sysmsg.valuecnt == 0) {

                sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
                INPUT2_count = 1;
                headcarpositoon.position = INPUT2_count;
                node.position = INPUT2_count;
                node.interval = headcarpositoon.interval;
                vphoto_add_to_slaver_position_queue(node);
                headcarpositoon.interval = 0;
//                AddSendMsgToQueue(SEND_MSG_CAR2WCS_CARPOSITION_TYPE);
                can_bus_send_photo_electricity(0, 0, INPUT2_count);

                if (wcs2carSysEnData.sys_en == 1)
                {
                    xialiao_process();
                }
            }
        }
    }
    else {
        if (sysmsg.photoreset == INVALUE) {
            sysmsg.photoreset = VALUE;
            sysmsg.slaverrst = VALUE;
            sysmsg.pcslaverrst = VALUE;

            sysmsg.resetcnt = PHOTO_RESET_MIN;
            sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
            INPUT2_count = tempcnt;
            headcarpositoon.position = INPUT2_count;
            node.position = INPUT2_count;
            node.interval = headcarpositoon.interval;
            vphoto_add_to_slaver_position_queue(node);
            headcarpositoon.interval = 0;
//            AddSendMsgToQueue(SEND_MSG_CAR2WCS_CARPOSITION_TYPE);
            can_bus_send_photo_electricity(0, 0, INPUT2_count);

            if (wcs2carSysEnData.sys_en == 1)
            {
                xialiao_process();
            }
        }
        else {
            if (sysmsg.valuecnt == 0) {
                sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
                if (INPUT2_count < max_guandian_count_u16)
                {
                    sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
                    if (slavefirstreset == VALUE) {
                        INPUT2_count++;
                    }
                    else {
                        INPUT2_count = tempcnt;
                    }
                    slavefirstreset = VALUE;

                    headcarpositoon.position = INPUT2_count;
                    node.position = INPUT2_count;
                    node.interval = headcarpositoon.interval;
                    vphoto_add_to_slaver_position_queue(node);
                    headcarpositoon.interval = 0;
//                    AddSendMsgToQueue(SEND_MSG_CAR2WCS_CARPOSITION_TYPE);
                    can_bus_send_photo_electricity(0, 0, INPUT2_count);

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
                        //                        if (s_LED_Interval == 0)
                        //                            s_LED_Interval = 400;

                        guangdian_error = 1;
                        INPUT2_count_error |= 0x8000;//多光电
                        moto_statue_comm_en = 1;
                    }
                }
            }
        }
    }
}

void photo_deal_with_cnt_photo(void)
{
    u8 input = INPUT2_STATUE;

    if (sysmsg.resetcnt != 0) {
        sysmsg.resetcnt--;
    }
    if (sysmsg.valuecnt != 0) {
        sysmsg.valuecnt--;
    }

    if (upload_photoone_cnt != 0) {
        upload_photoone_cnt--;
    }
    if (upload_phototwo_cnt != 0) {
        upload_phototwo_cnt--;
    }
    if (upload_photolast_cnt != 0) {
        upload_photolast_cnt--;
    }
    headcarpositoon.interval++;

    if (port_info[0].input_state != input
        && port_info[0].input_confirm_times == 0)
    {
        port_info[0].input_middle_state = input;
    }
    if (port_info[0].input_middle_state == input
        && port_info[0].input_middle_state != port_info[0].input_state)
    {
        port_info[0].input_confirm_times++;
        if (port_info[0].input_confirm_times > PHOTO_CNT_FILTER)// 2ms
        {
            port_info[0].input_state = port_info[0].input_middle_state;
            port_info[0].input_confirm_times = 0;
            if (port_info[0].input_state == 1)
            {
                port_info[0].input_flag = INPUT_TRIG_HIGH;
                port_info[0].input_trig_mode = INPUT_TRIG_UP;
                //                Photoeletricity_Process(INT_OPT_INC);
            }
            else
            {
                port_info[0].input_flag = INPUT_TRIG_LOW;
            }
        }
    }
    else
    {
        port_info[0].input_middle_state = port_info[0].input_state;
        port_info[0].input_confirm_times = 0;
    }

    if (port_info[0].input_trig_mode == INPUT_TRIG_UP) {
        port_info[0].input_trig_mode = INPUT_TRIG_NULL;
        Photoeletricity_Process(INT_OPT_INC);
    }

}


void photo_deal_with_reset_photo(void)
{
    u8 input = INPUT3_STATUE;

    if (port_info[1].input_state != input
        && port_info[1].input_confirm_times == 0)
    {
        port_info[1].input_middle_state = input;
    }
    if (port_info[1].input_middle_state == input
        && port_info[1].input_middle_state != port_info[1].input_state)
    {
        port_info[1].input_confirm_times++;
        if (port_info[1].input_confirm_times > PHOTO_RESET_FILTER)// 2ms
        {
            port_info[1].input_state = port_info[1].input_middle_state;
            port_info[1].input_confirm_times = 0;
            if (port_info[1].input_state == 1)
            {

                port_info[1].input_flag = INPUT_TRIG_HIGH;
                Photoeletricity_Process(INT_OPT_RST);
            }
            else
            {
                port_info[1].input_flag = INPUT_TRIG_LOW;
            }
        }
    }
    else
    {
        port_info[1].input_middle_state = port_info[1].input_state;
        port_info[1].input_confirm_times = 0;
    }

}

void photo_trig_config(void)
{
    u8 input1 = RESET_PHOTO_IN;
    u8 input2 = CNT_PHOTO_IN;
    sphotobdcnt node;

    phototrig.interval++;

    if (phototrig.valuecnt != 0) {
        phototrig.valuecnt--;
    }

    if (port_info[2].input_state != input1
        && port_info[2].input_confirm_times == 0)
    {
        port_info[2].input_middle_state = input1;
    }
    if (port_info[2].input_middle_state == input1
        && port_info[2].input_middle_state != port_info[2].input_state)
    {
        port_info[2].input_confirm_times++;
        if (port_info[2].input_confirm_times > PHOTO_RESET_FILTER)// 5ms
        {
            port_info[2].input_state = port_info[2].input_middle_state;
            port_info[2].input_confirm_times = 0;
            if (port_info[2].input_state == 1)
            {
                port_info[2].input_flag = INPUT_TRIG_HIGH;
                port_info[2].input_trig_mode = INPUT_TRIG_UP;
            }
            else
            {
                port_info[2].err_cnt = 0;
                port_info[2].input_flag = INPUT_TRIG_LOW;
            }
        }
    }
    else
    {
        port_info[2].input_middle_state = port_info[2].input_state;
        port_info[2].input_confirm_times = 0;
    }

    //  可以判断 光电报警 预留
    if (port_info[2].input_state == 1) {
        port_info[2].err_cnt++;
        if (port_info[2].err_cnt >= 5000) {
            port_info[2].err_cnt = 5000;
        }
    }

    if (port_info[2].input_trig_mode == INPUT_TRIG_UP) {
        port_info[2].input_trig_mode = INPUT_TRIG_NULL;

        if (phototrig.photoreset == INVALUE) {
            phototrig.photoreset = VALUE;
        }
        phototrig.curposition = 0;
        phototrig.resetlost = 0;
    }




    if (port_info[3].input_state != input2
        && port_info[3].input_confirm_times == 0)
    {
        port_info[3].input_middle_state = input2;
    }
    if (port_info[3].input_middle_state == input2
        && port_info[3].input_middle_state != port_info[3].input_state)
    {
        port_info[3].input_confirm_times++;
        if (port_info[3].input_confirm_times > PHOTO_CNT_FILTER)// 5ms
        {
            port_info[3].input_state = port_info[3].input_middle_state;
            port_info[3].input_confirm_times = 0;
            if (port_info[3].input_state == 1)
            {
                port_info[3].input_trig_mode = INPUT_TRIG_UP;
            }
            else
            {
                port_info[3].err_cnt = 0;
            }
        }
    }
    else
    {
        port_info[3].input_middle_state = port_info[3].input_state;
        port_info[3].input_confirm_times = 0;
    }

    //光电报警预留
    if (port_info[3].input_state == 1) {
        port_info[3].err_cnt++;
        if (port_info[3].err_cnt >= 5000) {
            port_info[3].err_cnt = 5000;
        }
    }

    if (port_info[3].input_trig_mode == INPUT_TRIG_UP) {
        port_info[3].input_trig_mode = INPUT_TRIG_NULL;

        if (phototrig.valuecnt == 0) {
            phototrig.valuecnt = PHOTO_INTERVAL_MIN;
            phototrig.curposition++;
            node.bdindex = 1;
            node.photocnt = phototrig.curposition;
            node.intervel = phototrig.interval;
            node.error = 0;
            vphoto_add_to_slaver_bd_queue(node);
            phototrig.interval = 0;
            phototrig.lostcount = 0;

            if ((phototrig.curposition > max_guandian_count_u16) && (wcs2carSysEnData.sys_en == 1))
            {
                phototrig.resetlost++;
            }
        }
    }
}