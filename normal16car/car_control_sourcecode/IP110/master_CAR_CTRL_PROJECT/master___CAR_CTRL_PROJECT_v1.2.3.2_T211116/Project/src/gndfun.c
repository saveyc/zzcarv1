#include "main.h"
#include "TCPclient.h"
#include "gndfun.h"

/* pcfunmsg*/
#define gndfunmsgSize  20
u16 gndfunmsgBuff[gndfunmsgSize];
MSG_SEND_QUEUE gndfunmsgQueue;


sgndcnt  gndcnt;
u8 gndcntflag = INVALUE;

u8 gndfirstreset = INVALUE;
u16 gndposcnt = 0;


void vgndfun_init_net(void)
{
    MSG_SEND_QUEUE* q;
    q = &gndfunmsgQueue;
    q->queue = gndfunmsgBuff;
    q->maxSize = gndfunmsgSize;
    q->front = q->rear = 0;
}

void vgndfun_add_net_Queue(MSG_SEND_QUEUE* q, u16 x)
{
    //队列满
    if ((q->rear + 1) % q->maxSize == q->front)
    {
        return;
    }
    q->rear = (q->rear + 1) % q->maxSize;       // 求出队尾的下一个位置
    q->queue[q->rear] = x;                      // 把x的值赋给新的队尾
}

u16  u16gndfun_get_msg_net_Queue(MSG_SEND_QUEUE* q)
{
    if (q->front == q->rear)
    {
        return 0;
    }
    q->front = (q->front + 1) % q->maxSize; // 使队首指针指向下一个位置
    return (u16)(q->queue[q->front]); // 返回队首元素
}

void vgndfun_add_gndfunmsg(u16 value)
{
    vgndfun_add_net_Queue(&gndfunmsgQueue, value);
}

u16 u16gndfun_getmsg_gndQueue(void)
{
    return u16gndfun_get_msg_net_Queue(&gndfunmsgQueue);
}

// 接收gnd发送的头车位置
void vgndfun_recv_carpos_msg(u8* point)
{
    sphotobdcnt  bdcnt;
    u16 temp = 0;

    temp = (point[0] | (point[1] << 8));

    if (wcs2carSysEnData.sys_en == 1) {

        if (temp < gndcnt.photocnt) {
            if (temp > 1) {
                gndtrig.cntlost = 1;
            }
        }
        else if (temp > (gndcnt.photocnt + 1)) {
            gndtrig.cntlost = 1;
        }

        if (temp > max_guandian_count_u16) {
            gndtrig.cntlost = 2;
        }
    }

    gndcnt.photocnt = (point[0] | (point[1] << 8));
    gndcnt.intervel = point[2] | (point[3] << 8);

    bdcnt.bdindex = 0xAA;
    bdcnt.intervel = gndcnt.intervel;
    bdcnt.photocnt = gndcnt.photocnt;
    bdcnt.error = gndtrig.cntlost;
    gndtrig.cntlost = 0;
    vphoto_add_to_slaver_bd_queue(bdcnt);
    gndposcnt = bdcnt.photocnt;
    sysmsg.lostcount = 0;

    gndtrig.lostcount = 0;

    if (gndcnt.photocnt != INPUT2_count) {
        gndcntflag = VALUE;
    }
    else {
        gndfirstreset = VALUE;
    }
}


u8 u8gndfun_recvmsg_check(u8* point, u16 len)
{
    u16 i = 0;
    u8  sum = 0;

    if ((point[0] != 0xAA) || (point[1] != 0xAA))
        return 0;
    if ((point[6] | point[7] << 8) != len)
        return 0;
    sum = point[9];
    //    for (i = 1; i < len - 9; i++)
    //    {
    //        sum ^= point[9 + i];
    //    }
    //    if (sum != point[8])
    //        return 0;

    return 1;
}


void vgndfun_recvmsg_process(u8* point)
{
    MSG_HEAD_DATA* head = (MSG_HEAD_DATA*)point;
    u16 datalen = 0;

    datalen = head->MSG_LENGTH;

    switch (head->MSG_TYPE)
    {
    case RECV_MSG_GND2BD_CARPOS_TYPE:
        vgndfun_recv_carpos_msg(point + sizeof(MSG_HEAD_DATA));
        break;
    default:
        break;
    }
}


void vgndfun_recvmessage_from_sever(u8* point, u16* len)
{
    if (u8gndfun_recvmsg_check(point, *len) == 0)
    {
        *len = 0;
        return;
    }
    vgndfun_recvmsg_process(point);
    *len = 0;
}


void vgndfun_reply_recv_msg(u8* buf, u16* len, u16 type)
{
    u8 sum = (type & 0xFF) ^ ((type >> 8) & 0xFF);

    buf[0] = 0xAA;
    buf[1] = 0xAA;
    buf[2] = 0x01;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0x0B;
    buf[7] = 0x00;
    buf[8] = sum;
    buf[9] = type & 0xFF;
    buf[10] = (type >> 8) & 0xFF;

    *len = 11;
}

void vgndfun_send_message_to_sever(void)
{
    u16 msg_type;

    if (tcp_client_list[1].tcp_send_en == 0)//tcp发送成功或没在发送
    {
        msg_type = u16gndfun_getmsg_gndQueue();
    }
    else//tcp正在发送
    {
        return;
    }

    switch (msg_type)
    {
    case REPLY_RECV_MSG_GND2BD_CARPOS_TYPE:
        vgndfun_reply_recv_msg(&(tcp_client_list[1].tcp_send_buf[0]), &(tcp_client_list[1].tcp_send_len), msg_type);
        tcp_client_list[1].tcp_send_en = 1;
        break;
    default:
        break;
    }
}



//use as a main photo

void vgnd_deal_with_carpos_process(void)
{
  
    u16 i =0;
    u16 tempcnt = 0;
    sCar2Wcs_headcar_positon node;
    if (gndtrig.valuecnt != 0) {
        gndtrig.valuecnt--;
    }

    if (gndcntflag == INVALUE) {
        return;
    }
    gndcntflag = INVALUE;

    if (gndcnt.photocnt == INPUT2_count) {
        return;
    }

    tempcnt = (gndcnt.photocnt + max_guandian_count_u16) % max_guandian_count_u16;
    if (tempcnt == 0) {
        tempcnt = max_guandian_count_u16;
    }


    if (tempcnt == 1) {
        if (sysmsg.resetcnt == 0) {
            if (sysmsg.photoreset == INVALUE) {
                sysmsg.photoreset = VALUE;
                sysmsg.slaverrst = VALUE;
                sysmsg.pcslaverrst = VALUE;
            }

            if (INPUT2_count > max_guandian_count_u16) {
                if (wcs2carSysEnData.sys_en == 1)
                {

                    guangdian_error = 1;
                    INPUT2_count_error |= 0x8000;//多光电
                    moto_statue_comm_en = 1;
                }
            }

            INPUT2_count = tempcnt;
            sysmsg.resetcnt = PHOTO_RESET_MIN;
            sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
            node.position = INPUT2_count;
            node.interval = headcarpositoon.interval;
            vphoto_add_to_slaver_position_queue(node);
            can_bus_send_photo_electricity(0, 0, INPUT2_count);
            if (wcs2carSysEnData.sys_en == 1)
            {
                xialiao_process();
            }

        }
        else {
            if (gndtrig.valuecnt == 0) {
                INPUT2_count = 1;
                sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
                node.position = INPUT2_count;
                node.interval = headcarpositoon.interval;
                gndtrig.valuecnt = PHOTO_INTERVAL_MIN;
                vphoto_add_to_slaver_position_queue(node);
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
            //                INPUT2_count = tempcnt;
            INPUT2_count = tempcnt;
            sysmsg.resetcnt = PHOTO_RESET_MIN;
            sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
            node.position = INPUT2_count;
            node.interval = headcarpositoon.interval;
            vphoto_add_to_slaver_position_queue(node);
            //            vpcfun_addto_sdsfunmsg(SEND_MSG_BD2PC_CAR_POSITION_TYPE);
            if (INPUT2_count <= max_guandian_count_u16) {
                can_bus_send_photo_electricity(0, 0, INPUT2_count);
                if (wcs2carSysEnData.sys_en == 1)
                {
                    xialiao_process();
                }
            }

        }
        else {
            if (gndtrig.valuecnt == 0) {
                INPUT2_count = tempcnt;
                sysmsg.valuecnt = PHOTO_INTERVAL_MIN;
                gndtrig.valuecnt = PHOTO_INTERVAL_MIN;
                node.position = INPUT2_count;
                node.interval = headcarpositoon.interval;
                vphoto_add_to_slaver_position_queue(node);
//                if (INPUT2_count <= max_guandian_count_u16) {
                    can_bus_send_photo_electricity(0, 0, INPUT2_count);
//                }
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
        }
    }


}


