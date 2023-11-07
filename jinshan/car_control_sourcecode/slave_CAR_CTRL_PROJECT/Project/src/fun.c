#include "main.h"
#include "TCPclient.h"

sCar2WCS_online_Data car2WcsOnlineData;


sLoad_info Load_info_t;
sUnload_info Unload_info_t;
sWCS2Car_Para_Data wcs2CarParaData;
sWCS2Car_SYS_EN_Data    wcs2carSysEnData;
sWCS2Car_Unload_run_Data    sWCS2CarUnloadRunData;
#define msgSendQueueSize  100
u16 msgQueueBuff[msgSendQueueSize];
MSG_SEND_QUEUE msgSendQueue;
u32  WIFI_test = 0;
u8 WCS_reply_timer_out = 0;

u32 s_speed_zhuxian_u32 = 0;

u8 sys_moto_statue[MAX_GUANGDIAN_COUNT];

u8 sys_moto_statue_send_en = 0;
u16 reply_wcs_type = 0;
u8 index_test_load = 0;
u32 reply_index_error = 0;
u32 recive_wcs_count = 0;
u8 udp_send_en = 0;
/****************************************************************

****************************************************************/
void WIFI_test_process(void)
{
    u8 send_len = 0;
    u8 i;
    char str[25];
#ifdef SLAVE
    TX_DEBUG_485;
#endif
    /*    send_len = sprintf(str, "L:%d " , INPUT2_L_timer);
    #ifdef SLAVE
            for(i=0 ; i<send_len ; i++)
            {
            USART_SendData(DEBUG_USART, str[i]);
            while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
            }
    #else
            DEBUG_process(str,send_len);
    #endif
        send_len = sprintf(str, "H:%d " , INPUT2_H_timer);
    #ifdef SLAVE
            for(i=0 ; i<send_len ; i++)
            {
            USART_SendData(DEBUG_USART, str[i]);
            while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
            }
    #else
            DEBUG_process(str,send_len);
    #endif*/
    send_len = sprintf(str, "E:%d " , wcs2carSysEnData.sys_en);
    //send_len = sprintf(str, "S:%d " , wcs2CarParaData.exit_num);
#ifdef SLAVE
    for(i = 0 ; i < send_len ; i++)
    {
        USART_SendData(DEBUG_USART, str[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
#else
    DEBUG_process(str, send_len);
#endif
    send_len = sprintf(str, "S:%d " , s_speed_zhuxian_u32);
    //send_len = sprintf(str, "S:%d " , wcs2CarParaData.exit_num);
#ifdef SLAVE
    for(i = 0 ; i < send_len ; i++)
    {
        USART_SendData(DEBUG_USART, str[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
#else
    DEBUG_process(str, send_len);
#endif
    send_len = sprintf(str, "D:%d " , recive_wcs_count & 0x00ffffff);
    //send_len = sprintf(str, "S:%d " , wcs2CarParaData.exit_num);
#ifdef SLAVE
    for(i = 0 ; i < send_len ; i++)
    {
        USART_SendData(DEBUG_USART, str[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
#else
    DEBUG_process(str, send_len);
#endif

    send_len = sprintf(str, "G:%d\n" , INPUT2_count);
#ifdef SLAVE
    for(i = 0 ; i < send_len ; i++)
    {
        USART_SendData(DEBUG_USART, str[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
#else
    DEBUG_process(str, send_len);
#endif
#ifdef SLAVE
    RX_DEBUG_485;
#endif

}
/************************************************************************

************************************************************************/

void initQueue(MSG_SEND_QUEUE *q, u16 ms)
{
    q->maxSize = ms;
    q->queue = msgQueueBuff;
    q->front = q->rear = 0;
}
void enQueue(MSG_SEND_QUEUE *q, u16 x)
{
    //队列满
    if((q->rear + 1) % q->maxSize == q->front)
    {
        return;
    }
    q->rear = (q->rear + 1) % q->maxSize; // 求出队尾的下一个位置
    q->queue[q->rear] = x; // 把x的值赋给新的队尾
}
u16 outQueue(MSG_SEND_QUEUE *q)
{
    //队列空
    if(q->front == q->rear)
    {
        return 0;
    }
    q->front = (q->front + 1) % q->maxSize; // 使队首指针指向下一个位置
    return q->queue[q->front]; // 返回队首元素
}
void InitSendMsgQueue(void)
{
    initQueue(&msgSendQueue, msgSendQueueSize);
}
void AddSendMsgToQueue(u16 msg)
{
    enQueue(&msgSendQueue, msg);
}
u16 GetSendMsgFromQueue(void)
{
    return (outQueue(&msgSendQueue));
}

u8 recv_msg_check(u8 *point, u16 len)
{
    u16 i = 0;
    u8  sum = 0;

    if((point[0] != 0xAA) || (point[1] != 0xAA))
        return 0;
    if((point[6] | point[7] << 8) != len)
    {
        return 0;
    }
    sum = point[9];
    for(i = 1 ; i < len - 9 ; i++)
    {
        sum ^= point[9 + i];
    }
    if(sum != point[8])
        return 0;

    return 1;
}
/******************************************************

******************************************************/


/*******************************************************************************

********************************************************************************/
void  xiaoliao_exit_process(u8 *point)
{
    u16 str_addr, len, i, tmp, j;
    if( *(point) == 0 )
    {
        //  xialiao
        str_addr = *(point + 2);
        str_addr <<= 8;
        str_addr |= *(point + 1);
        len = *(point + 4);
        len <<= 8;
        len |= *(point + 3);
        if( ( str_addr + len) > EXIT_NUM )
        {
            return;
        }
        if( ( str_addr + len) > wcs2CarParaData.exit_num )
        {
            wcs2CarParaData.exit_num = ( str_addr + len);
        }
        for(i = 0; i < len; i++)
        {
            tmp = *(point + 6 + i * 5);
            tmp <<= 8;
            tmp |= *(point + 5 + i * 5);
            wcs2CarParaData.exit_position[str_addr + i] = tmp;
            wcs2CarParaData.exit_direction[str_addr + i] = *(point + 7 + i * 5);
            wcs2CarParaData.exit_speed[str_addr + i] = *(point + 8 + i * 5);
            wcs2CarParaData.exit_stop[str_addr + i] = *(point + 9 + i * 5);
            for(j = 0; j < MOTO_MAX_COUNT; j++)
            {
                if( moto_para[j].xialiao_postion != 0xffff )
                {
                    if( ( moto_para[j].xialiao_postion == wcs2CarParaData.exit_position[str_addr + i] )
                            && ( moto_para[j].fangxiang == wcs2CarParaData.exit_direction[str_addr + i]) )
                    {
                        if( wcs2CarParaData.exit_stop[str_addr + i] == 1 )
                        {
                            moto_para[j].xialiao_postion = 0xffff;
                            //break;
                        }
                    }
                }
            }
        }
        //renew_xialiaokou_list();
    }
    else
    {
        //load
        str_addr = *(point + 2);
        str_addr <<= 8;
        str_addr |= *(point + 1);
        len = *(point + 4);
        len <<= 8;
        len |= *(point + 3);
        if( ( str_addr + len) > LOAD_NUM )
        {
            return;
        }
        for(i = 0; i < len; i++)
        {
            tmp = *(point + 6 + i * 9);
            tmp <<= 8;
            tmp |= *(point + 5 + i * 9);
            wcs2CarParaData.load_position[str_addr + i] = tmp;
            wcs2CarParaData.load_direction[str_addr + i] = *(point + 7 + i * 9);
            wcs2CarParaData.a_u16[str_addr + i] = (*(point + 8 + i * 9)) | ((*(point + 9 + i * 9)) << 8);
            wcs2CarParaData.b_u16[str_addr + i] = (*(point + 10 + i * 9)) | ((*(point + 11 + i * 9)) << 8);
            wcs2CarParaData.c_s16[str_addr + i] = (*(point + 12 + i * 9)) | ((*(point + 13 + i * 9)) << 8);
        }
    }
}

/*******************************************************************************

********************************************************************************/
void  xiaoliao_IO_exit_process(u8 *point)
{
    u16 i, j; //len,,tmp
    sIO_Info sIO_Info_t;
    u16 addr_num = point[0]|(point[1]<<8);
    
    if(addr_num < 1)
    {
        return;
    }

    for(i = 0; i < addr_num; i++)
    {
        sIO_Info_t = *((sIO_Info *)(point + 2 + sizeof(sIO_Info) * i));
        wcs2CarParaData.exit_stop[sIO_Info_t.star_addr] = 1;//sIO_Info_t.exit_stop;

        for(j = 0; j < MOTO_MAX_COUNT; j++)
        {
            if( moto_para[j].xialiao_postion != 0xffff )
            {
                if( ( moto_para[j].xialiao_postion == wcs2CarParaData.exit_position[sIO_Info_t.star_addr] )
                        && ( moto_para[j].fangxiang == wcs2CarParaData.exit_direction[sIO_Info_t.star_addr]) )
                {
                    if( wcs2CarParaData.exit_stop[sIO_Info_t.star_addr] == 1 )
                    {
                        moto_para[j].xialiao_postion = 0xffff;
                        //break;
                    }
                }
            }
        }

    }
}
/*****************************************************************

*****************************************************************/
void reply_error_process(u8 *point, u16 len, u8 error)
{
    u32 tmp;
    // u8 send_len = 0;
    // char str[25];

    tmp = error;
    MSG_HEAD_DATA *head = (MSG_HEAD_DATA *)point;
    head->MSG_ID &= 0x00ffffff;
    head->MSG_ID |= (tmp << 24);
    WCS_TCP_reply_process(point, len);
    recive_wcs_count++;
#ifdef  WIFI_TEST
    if( WCS_ACK_timer > 100 )
    {
        send_len = sprintf(str, "TD:%d\n" , WCS_ACK_timer);
        //DEBUG_process(str,send_len);
    }
#endif
}
/****************************************************************

****************************************************************/
#ifdef _UDP_DEBUG_
u8 debug_info[500];
#endif
void recv_msg_process(u8 *point)
{
    
}
void recv_message_from_sever(u8 *point, u16 *len)
{
    // u8 str[40];
    //DEBUG_process(point,*len);
    if(recv_msg_check(point, *len) == 0)
    {
        //reply_error_process(point,*len,1);
        *len = 0;
        return;
    }

    recv_msg_process(point);
    *len = 0;
}
void reply_recv_msg(u8 *buf, u16 *len, u16 type, u32 index)
{
    u8 sum = (type & 0xFF) ^ (type >> 8);

    buf[0] = 0xAA;
    buf[1] = 0xAA;
    //buf[2] = 0x01;
    //buf[3] = 0x00;
    //buf[4] = 0x00;
    //buf[5] = 0x00;
    buf[2] = index & 0xFF;
    buf[3] = (index >> 8) & 0xFF;
    buf[4] = (index >> 16) & 0xFF;
    buf[5] = (index >> 24) & 0xFF;
    buf[6] = 0x0F;
    buf[7] = 0x00;
    buf[8] = sum;
    buf[9] = type & 0xFF;
    buf[10] = (type >> 8) & 0xFF;

    buf[11] = recive_wcs_count & 0xFF;
    buf[12] = (recive_wcs_count >> 8) & 0xFF;
    buf[13] = (recive_wcs_count >> 16) & 0xFF;
    buf[14] = (recive_wcs_count >> 24) & 0xFF;
    *len = 15;
}

/***************************************************************************************

***************************************************************************************/
void reply_recv_msg_error_statue(u8 *buf, u16 *len, u16 type, u32 index)
{
    u8 sum; //= (type&0xFF)^(type>>8);
    u16 i;
    buf[0] = 0xAA;
    buf[1] = 0xAA;
    buf[2] = index & 0xFF;
    buf[3] = (index >> 8) & 0xFF;
    buf[4] = (index >> 16) & 0xFF;
    buf[5] = (index >> 24) & 0xFF;
    buf[6] = 0x0F;
    buf[7] = 0x00;
    //buf[8] = sum;
    buf[9] = type & 0xFF;
    buf[10] = (type >> 8) & 0xFF;

    buf[11] = recive_wcs_count & 0xFF;
    buf[12] = (recive_wcs_count >> 8) & 0xFF;
    buf[13] = (recive_wcs_count >> 16) & 0xFF;
    buf[14] = (recive_wcs_count >> 24) & 0xFF;
    *len = 15;
    //moto_statue_comm_en = 1;
    //statue_car_num = 2;
    //statue_car_count = 3;
    //故障区
    //statue_car_num = car_num;
    if( (moto_statue_comm_en == 1) && (statue_car_num != 0) && ( (statue_car_num + statue_car_count ) < max_guandian_count_u16 ) )
    {
        buf[15] = statue_guangdian_num & 0xFF;
        buf[16] = (statue_guangdian_num >> 8) & 0xFF;
        buf[15 + 2] = statue_car_count;
        *len += (1 + 2);
        for(i = 0; i < statue_car_count; i++)
        {
            buf[2 + 16 + 3 * i] = (statue_car_num + i);
            buf[2 + 17 + 3 * i] = (statue_car_num + i) >> 8;
            buf[2 + 18 + 3 * i] = sys_moto_statue[ statue_car_num - 1 + i];
            *len += 3;
        }
        moto_statue_comm_en = 0;
    }

    sum = buf[9];
    for(i = 1 ; i < *len - 9 ; i++)
    {
        sum ^= buf[9 + i];
    }
    buf[8] = sum;
    buf[6] = *len;
    buf[7] = *len >> 8;
}

void send_msg_car2wcs_heart(u8 *buf, u16 *len, u16 type)
{
    u8 sum = (type & 0xFF) ^ (type >> 8);

    buf[0] = 0xAA;
    buf[1] = 0xAA;
    buf[2] = 0x01;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x0B;
    buf[7] = 0x00;
    buf[8] = sum;
    buf[9] = type & 0xFF;
    buf[10] = (type >> 8) & 0xFF;
    *len = 11;
}



#define VERSION(a, b, c, d) ((((a << 4) | (b)) << 8) | (c << 4) | ( d))
void send_msg_car2wcs_online_cmd(u8 *buf, u16 *len, u16 type)
{
    //  u8 sum;
    u16 sendlen;
    // u16 i;

    sendlen = sizeof(sCar2WCS_online_Data);

    car2WcsOnlineData.head_t.tag_u16          = 0xaaaa;
    car2WcsOnlineData.head_t.sequence_num_u32 = 1;
    car2WcsOnlineData.head_t.length_u16       = sendlen;
    car2WcsOnlineData.head_t.cmd_u16          = type;
    car2WcsOnlineData.car_version = VERSION(1, 2, 0, 1);//(((1 << 4) | (2)) << 8) | (0 << 4) | ( 1);
    car2WcsOnlineData.head_t.checksum_u8      = (type & 0xff) ^ (type >> 8) ^ (car2WcsOnlineData.car_version & 0xff) ^ (car2WcsOnlineData.car_version >> 8);

    memcpy(buf, &car2WcsOnlineData, sendlen);

    *len = sendlen;
}
void send_message_to_sever(void)
{
    u16 msg_type;
    //u8 send_len = 0;
    //  char str[25];
    if( TCP_send_dely != 0 )
    {
        return;
    }
    if(tcp_client_list[0].tcp_send_en == 0)//tcp发送成功或没在发送
    {
        msg_type = GetSendMsgFromQueue();
    }
    else//tcp正在发送
    {
        return;
    }

    switch(msg_type)
    {
    case REPLY_RECV_MSG_WCS2CAR_LOAD_OPT_RUNTIME_PARAS:
    case REPLY_RECV_MSG_WCS2CAR_CMD_TYPE:
    case REPLY_RECV_MSG_WCS2CAR_PARA_TYPE:
    case REPLY_RECV_MSG_WCS2CAR_FENJIAN_EN_TYPE:
    case REPLY_RECV_MSG_WCS2CAR_UNLOAD_RUN_DATA:
        reply_recv_msg(&(tcp_client_list[0].tcp_send_buf[0]), &(tcp_client_list[0].tcp_send_len), msg_type, reply_index_error);
        tcp_client_list[0].tcp_send_en = 1;
#ifdef  WIFI_TEST
        if( WCS_ACK_timer > 100 )
        {
            send_len = sprintf(str, "TD:%d\n" , WCS_ACK_timer);
            DEBUG_process(str, send_len);
        }
#endif
        break;
    case REPLY_RECV_MSG_WCS2CAR_LOAD_CMD_TYPE:
        reply_recv_msg_error_statue(&(tcp_client_list[0].tcp_send_buf[0]), &(tcp_client_list[0].tcp_send_len), msg_type, reply_index_error);
        tcp_client_list[0].tcp_send_en = 1;
#ifdef  WIFI_TEST
        if( WCS_ACK_timer > 100 )
        {
            send_len = sprintf(str, "TD:%d\n" , WCS_ACK_timer);
            DEBUG_process(str, send_len);
        }
#endif
        break;
    case SEND_MSG_CAR2WCS_ONLINE_TYPE:
        send_msg_car2wcs_online_cmd(&(tcp_client_list[0].tcp_send_buf[0]), &(tcp_client_list[0].tcp_send_len), msg_type);
        tcp_client_list[0].tcp_send_en = 1;
        WCS_reply_timer_out = 100;
        break;
    default:
        if( tcp_client_list[0].tcp_client_statue == CLIENT_CONNECT_OK)
        {
            if( heart_dely == 0 )
            {
                send_msg_car2wcs_heart(&(tcp_client_list[0].tcp_send_buf[0]), &(tcp_client_list[0].tcp_send_len), SEND_MSG_CAR2WCS_HEART_TYPE);
                tcp_client_list[0].tcp_send_en = 1;
                heart_dely = HEART_DELAY;
            }
        }
        break;
    }
}
extern u8 Debug_send_buff[DEBUG_BUFF_SIZE];

void Broadcast_Photoeletricity(u8 level, u8 error_flag, uint16_t INPUT2_count, u32 zhuxian_speed)
{

    //return ;
    u8  i = 0, crc = 0;//err,
    // u8 LumMod_Tx_Index ;
    TX_DEBUG_485
    Debug_send_buff[0] = 'g';
    Debug_send_buff[1] = 'd';
    Debug_send_buff[2] = level | (error_flag << 1);
    Debug_send_buff[3] = INPUT2_count & 0xff;
    Debug_send_buff[4] = (INPUT2_count >> 8) & 0xff;
    *((u32 *) (&Debug_send_buff[5])) = zhuxian_speed;

    crc = Debug_send_buff[0];
    for(i = 1; i < 9; i++)
    {
        crc = crc ^ Debug_send_buff[i];
    }

    Debug_send_buff[9] = crc;


    DMA_Cmd(DMA2_Channel5, DISABLE);
    DMA2_Channel5->CNDTR = 10; // 设置要发送的字节数目
    DMA_Cmd(DMA2_Channel5, ENABLE);        //开始DMA发送
}
