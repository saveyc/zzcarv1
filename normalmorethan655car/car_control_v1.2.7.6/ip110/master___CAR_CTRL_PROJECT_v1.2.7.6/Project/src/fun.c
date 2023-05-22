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
//u16 reply_wcs_type = 0;
u8 index_test_load = 0;
u32 reply_index_error = 0;
u32 recive_wcs_count = 0;
u8 udp_send_en = 0;



sFunslaverheart           slaverheartmsg;

sCar2Wcs_headcar_positon     headcarpositoon;

u8 Isphotovalue = 0;
u8 Isplcvalue = 0;

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
    u32 str_addr, len, i, tmp, j;
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
            tmp = point[5+i*7]|(point[6+i*7]<<8)|(point[7+i*7]<<16)|(point[8+i*7]<<24);
            wcs2CarParaData.exit_position[str_addr + i] = tmp;
            wcs2CarParaData.exit_direction[str_addr + i] = *(point + 9 + i * 7);
            wcs2CarParaData.exit_speed[str_addr + i] = *(point + 10 + i * 7);
            wcs2CarParaData.exit_stop[str_addr + i] = *(point + 11 + i * 7);
            for(j = 0; j < MOTO_MAX_COUNT; j++)
            {
                if( moto_para[j].xialiao_postion != 0xffffffff )
                {
                    if( ( moto_para[j].xialiao_postion == wcs2CarParaData.exit_position[str_addr + i] )
                            && ( moto_para[j].fangxiang == wcs2CarParaData.exit_direction[str_addr + i]) )
                    {
                        if( wcs2CarParaData.exit_stop[str_addr + i] == 1 )
                        {
                            moto_para[j].xialiao_postion = 0xffffffff;
                            //break;
                        }
                    }
                }
            }
        }

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
    u16 i, j;//len,,tmp,
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
            if( moto_para[j].xialiao_postion != 0xffffffff )
            {
                if( ( moto_para[j].xialiao_postion == wcs2CarParaData.exit_position[sIO_Info_t.star_addr] )
                        && ( moto_para[j].fangxiang == wcs2CarParaData.exit_direction[sIO_Info_t.star_addr]) )
                {
                    if( wcs2CarParaData.exit_stop[sIO_Info_t.star_addr] == 1 )
                    {
                        moto_para[j].xialiao_postion = 0xffffffff;
                        //break;
                    }
                }
            }
        }

    }
}

void check_car_cmd_process(u8* point)
{
    sCheckCar_CMD_Data cmd;
    u8 i;
    
    cmd = *((sCheckCar_CMD_Data *)point);
    
    for(i = 0; i < MOTO_MAX_COUNT; i++)
    {
        if( moto_para[i].xiaoche_num == cmd.car_num )
        {
            check_motor_flag = 1;
            moto_para[i].speed = cmd.speed;
            moto_para[i].fangxiang = cmd.direction;
            moto_para[i].juli = cmd.length;
            moto_para[i].dely = 0;
            moto_para[i].run_en = MOTO_OPT_SEND_PARA;
            moto_start_ctr = 1;
            break;
        }
    }
}
void recv_slaver_photo_config_process(u8* point)
{
    u8 i = 0;
    u8 num = 0;

    Isphotovalue = 0;
    Isplcvalue = point[0];
    num = point[1];

    for (i = 0; i < num; i++) {
        if (point[2 + i] == 1) {
            Isphotovalue = 1;
        }
    }
}

/*****************************************************************

*****************************************************************/
void reply_error_process(u8 *point, u16 len, u8 error)
{
    u32 tmp;
    // u8 send_len = 0;
    //  char str[25];

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


u32 s_Last_packet_seq = -1;


void recv_msg_process(u8 *point)
{
    u8  packet_count_u8, i = 0;
    u32 speed_tmp_u32 = 0;
    u8 protocol_error_u8 = 0;
    u16 head_size_u16 = 0, speed_size_u16 = 0, load_info_size_u16 = 0, unload_info_size = 0, k = 0;
    MSG_HEAD_DATA *head = (MSG_HEAD_DATA *)point;
    sLoad_platform_paras *pPlatform_runtime_paras = 0;
    WIFI_test++;

    if( head->MSG_TYPE == RECV_MSG_BOOT_CMD_TYPE )
    {
        BKP_WriteBackupRegister(BKP_DR8, 0x55);
        NVIC_SystemReset();
    }
    if( head->MSG_TYPE == REPLY_SEND_MSG_CAR2WCS_ONLINE_TYPE )
    {
        WCS_reply_timer_out = 0;
        udp_send_en = 1;
    }
    if( udp_send_en == 0 )
    {
        return;
    }

    switch(head->MSG_TYPE)
    {
    case RECV_MSG_WCS2CAR_ONLINE_TYPE: //WCS上线命令 0x1120
        udp_send_en = 0;
        AddSendMsgToQueue(SEND_MSG_CAR2WCS_ONLINE_TYPE);

        guangdian_error = 0;
        INPUT2_count_error = 0;
        moto_statue_comm_en = 0;
        
        motor_position_reset();//19/6/25 add

        //zongxian_uart_broadcast(point, RECV_MSG_WCS2CAR_ONLINE_TYPE);
        can_bus_send_wcs_msg(point, head->MSG_LENGTH);

        break;
    case RECV_MSG_WCS2CAR_FENJIAN_EN_TYPE: //分拣许可命令 0x1121
        //DEBUG_process(point,head->MSG_LENGTH);
        wcs2carSysEnData = *((sWCS2Car_SYS_EN_Data *)point);
        //zongxian_uart_broadcast(point, head->MSG_TYPE);
        can_bus_send_wcs_msg(point, head->MSG_LENGTH);
        reply_index_error = head->MSG_ID;
        recive_wcs_count++;
        check_motor_flag = 0;
        AddSendMsgToQueue(REPLY_RECV_MSG_WCS2CAR_FENJIAN_EN_TYPE);
        if (wcs2carSysEnData.sys_en == 1) {
            sysmsg.pcslaverrst = 1;
        }
        slavefirstreset = INVALUE;
        INPUT2_count_error = 0;
        guangdian_error = 0;
        sysmsg.cntlost = 0;
        phototrig.cntlost = 0;
        break;
    case RECV_MSG_WCS2CAR_LOAD_OPT_RUNTIME_PARAS: //上料运行时参数(a,b,c) 0x1132
        /*
         -------------------------------------------------------------------------------------------------
         |            Frame Head (11B)          | adjust_count |   load_runtime_paras (adjust_count*7)(B)|
         |--------------------------------------|--------------|-----------------------------------------|
         |0xaaaa|seq(4B)|len(2B)|crc(1B)|cmd(2B)|      1B      |    sLoad_platform_paras[adjust_count]   |
         -------------------------------------------------------------------------------------------------
        */

        packet_count_u8 = *(point + 11);

        if( (packet_count_u8 <= LOAD_NUM) && (packet_count_u8 > 0) )
        {
            for(i = 0; i < packet_count_u8; i++)
            {
                pPlatform_runtime_paras = (sLoad_platform_paras *) (point + 12 + i * sizeof(sLoad_platform_paras));
                if(pPlatform_runtime_paras->platform_index_u8 < LOAD_NUM)
                {
                    wcs2CarParaData.a_u16[pPlatform_runtime_paras->platform_index_u8] = pPlatform_runtime_paras->a_u16;
                    wcs2CarParaData.b_u16[pPlatform_runtime_paras->platform_index_u8] = pPlatform_runtime_paras->b_u16;
                    wcs2CarParaData.c_s16[pPlatform_runtime_paras->platform_index_u8] = pPlatform_runtime_paras->c_s16;
                }//if(pPlatform_runtime_paras->platform_index_u8 < LOAD_NUM)
            }//for(i = 0; i < packet_count_u8; i++)
        }// if( (packet_count_u8 <= LOAD_NUM) && (packet_count_u8 > 0) )

        //zongxian_uart_broadcast(point, head->MSG_TYPE);
        can_bus_send_wcs_msg(point, head->MSG_LENGTH);

        reply_index_error = head->MSG_ID;
        recive_wcs_count++;
        AddSendMsgToQueue(REPLY_RECV_MSG_WCS2CAR_LOAD_OPT_RUNTIME_PARAS);

        break;

    case RECV_MSG_WCS2CAR_LOAD_CMD_TYPE: //四包合一数据 0x1301
        //  -------------------------------------------------------------------------------------------------------------------------------------------------------------
        //  |            Frame Head (11B)          | zhuxian speed |          load_info (1+ 9*car_num B)|  unload_info  (1+ 9*car_num B)       | IO_info (2+ 2*num B)   |
        //  |--------------------------------------|---------------|------------------------------------|--------------------------------------|------------------------|
        //  |0xaaaa|seq(4B)|len(2B)|crc(1B)|cmd(2B)|   4B          |car_num(1B)| sCar_Load_Info[car_num]|car_num(1B)| sCar_Unload_Data[car_num]| num(2B) | io_addr[num] |
        //  -------------------------------------------------------------------------------------------------------------------------------------------------------------
        if(s_Last_packet_seq != head->MSG_ID)
        {
            protocol_error_u8 = 0;
            reply_index_error = head->MSG_ID;
            recive_wcs_count++;

            head_size_u16 = sizeof(MSG_HEAD_DATA);

            speed_tmp_u32 = *(u32 *)(point + head_size_u16);  //实际速度*100
            s_speed_zhuxian_u32 = speed_tmp_u32 * 10;  //转换成 mm/s
            speed_size_u16 = 4;

            Load_info_t.platform_num_u8 = *(u8 *)(point + head_size_u16 + speed_size_u16);
            if(Load_info_t.platform_num_u8 <= MAX_LOAD_PLATFORM_NUM)
            {
                memcpy((Load_info_t.car_load_info_t), (u8 *)(point + head_size_u16 + speed_size_u16 + 1), Load_info_t.platform_num_u8 * sizeof(sCar_Load_Info));
            }
            else
            {
                protocol_error_u8 = 1;
            }
            load_info_size_u16 = 1 + Load_info_t.platform_num_u8 * sizeof(sCar_Load_Info);

            Unload_info_t.unload_car_num_u8 = *(u8 *)(point + head_size_u16 + speed_size_u16 + load_info_size_u16);
            if(Unload_info_t.unload_car_num_u8 <= MAX_UNLOAD_CAR_NUM)
            {
                memcpy((Unload_info_t.car_unload_info_t), (u8 *)(point + 1 + head_size_u16 + speed_size_u16 + load_info_size_u16), Unload_info_t.unload_car_num_u8 * sizeof(sCar_unload_Data));
            }
            else
            {
                protocol_error_u8 = 1;
            }
            unload_info_size = 1 + Unload_info_t.unload_car_num_u8 * sizeof(sCar_unload_Data);

            for(k = 0; k < EXIT_NUM; k++)
                wcs2CarParaData.exit_stop[k] = 0;

            if( (!protocol_error_u8) && ((head->MSG_LENGTH ) > ( head_size_u16 + speed_size_u16 + load_info_size_u16 + unload_info_size)) )
            {
                if( ( ( head->MSG_LENGTH - ( head_size_u16 + speed_size_u16 + load_info_size_u16 + unload_info_size ) - 2 ) % 2 ) != 0 )
                {
                    reply_index_error |= 3 << 24;
                }
                xiaoliao_IO_exit_process(point + head_size_u16 + speed_size_u16 + load_info_size_u16 + unload_info_size );
            }
            else
            {
                reply_index_error |= 3 << 24;
            }

            can_bus_send_wcs_msg(point, head->MSG_LENGTH);
            
            AddSendMsgToQueue(REPLY_RECV_MSG_WCS2CAR_LOAD_CMD_TYPE);
            get_xialiao_num_process();
            get_load_num();
        }
        else
        {
            AddSendMsgToQueue(REPLY_RECV_MSG_WCS2CAR_LOAD_CMD_TYPE);
        }

        s_Last_packet_seq = head->MSG_ID;

        break;
    case RECV_MSG_WCS2CAR_PARA_TYPE: //下料口和供包台配置参数 0x1130
        reply_index_error = head->MSG_ID;
        recive_wcs_count++;
        xiaoliao_exit_process(point + 11);
        //DEBUG_process(point,head->MSG_LENGTH);
        AddSendMsgToQueue(REPLY_RECV_MSG_WCS2CAR_PARA_TYPE);
        //zongxian_uart_broadcast(point, head->MSG_TYPE);
        can_bus_send_wcs_msg(point, head->MSG_LENGTH);
        
        break;
    case RECV_MSG_WCS2CAR_UNLOAD_RUN_DATA: //下料配置参数 0x1135
        //DEBUG_process(point,head->MSG_LENGTH);
        sWCS2CarUnloadRunData = *((sWCS2Car_Unload_run_Data *)point);
        max_guandian_count_u16 = sWCS2CarUnloadRunData.car_num_u16;
        if( (max_guandian_count_u16 - XIAOCHE_START_NUM ) > 16 )
        {
            MOTO_MAX_COUNT = 16;
        }
        else
        {
            MOTO_MAX_COUNT = max_guandian_count_u16 - XIAOCHE_START_NUM + 1;
        }
        //zongxian_uart_broadcast(point, head->MSG_TYPE);
        can_bus_send_wcs_msg(point, head->MSG_LENGTH);
        reply_index_error = head->MSG_ID;
        AddSendMsgToQueue(REPLY_RECV_MSG_WCS2CAR_UNLOAD_RUN_DATA);
        break;
    case RECV_MSG_WCS2CAR_CHECK_CAR_TYPE: //测试小车命令 0x1133
        check_car_cmd_process(point);
        can_bus_send_wcs_msg(point, head->MSG_LENGTH);
        break;
    case RECV_MSG_SLAVER_PHOTO_CONF_TYPE:
        recv_slaver_photo_config_process(point + 11);
        can_bus_send_wcs_msg(point, head->MSG_LENGTH);
        AddSendMsgToQueue(REPLY_RECV_MSG_SLAVER_PHOTO_CONF_TYPE);
        break;
    default:
//        if( head->MSG_TYPE < 0x9000 )
//        {
//            reply_error_process(point, tcp_client_list[0].tcp_recev_len, 2);
//        }
        break;
    }
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
    if( (moto_statue_comm_en == 1) && (statue_car_num != 0) && ( (statue_car_num + statue_car_count - 1) <= max_guandian_count_u16 ) )
    {
        if(INPUT2_count_error & 0x2000)
        {
            buf[15] = 1;//   mastr find photoeletricity error ,warning rightnow
            buf[16] = 0x20;
        }
        else
        {
            buf[15] = statue_guangdian_num & 0xFF;
            buf[16] = (statue_guangdian_num >> 8) & 0xFF;
        }
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
    
    buf[2] = 0x01;//版本号
    buf[3] = 0x02;
    buf[4] = 0x02;
    buf[5] = 0x00;
    
    buf[6] = 0x0B;
    buf[7] = 0x00;
    buf[8] = sum;
    buf[9] = type & 0xFF;
    buf[10] = (type >> 8) & 0xFF;
    *len = 11;
}


//发送车载上线命令
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
    car2WcsOnlineData.car_version = VERSION(1, 2, 6, 6);//(((1 << 4) | (2)) << 8) | (0 << 4) | ( 1);
    car2WcsOnlineData.head_t.checksum_u8      = (type & 0xff) ^ (type >> 8) ^ (car2WcsOnlineData.car_version & 0xff) ^ (car2WcsOnlineData.car_version >> 8);

    memcpy(buf, &car2WcsOnlineData, sendlen);

    *len = sendlen;
}

void fun_head_car_position_cmd(u8* buf, u16* len, u16 type)
{
    u16 sendlen = 0;
    u8 sum;
    u8 i = 0;
    sCar2Wcs_headcar_positon* node = NULL;

    node = vphoto_get_from_positionqueue();

    if (sysmsg.cntlost != 0) {
        sendlen = 11 + 6;
        buf[9] = type & 0xFF;
        buf[10] = (type >> 8) & 0xFF;

        buf[11] = node->position & 0xFF;
        buf[12] = (node->position >> 8) & 0xFF;

        buf[13] = node->interval & 0xFF;
        buf[14] = (node->interval >> 8) & 0xFF;
        buf[15] = sysmsg.pcslaverrst;
        buf[16] = sysmsg.cntlost;

        headcarpositoon.interval = 0;



        sum = buf[9];

        for (i = 1; i < sendlen - 9; i++) {
            sum ^= buf[9 + i];
        }

        buf[0] = 0xAA;
        buf[1] = 0xAA;
        buf[2] = 0;
        buf[3] = 0;
        buf[4] = 0;
        buf[5] = 0;
        buf[6] = sendlen & 0xFF;
        buf[7] = (sendlen >> 8) & 0xFF;
        buf[8] = sum;

        *len = sendlen;

        sysmsg.pcslaverrst = INVALUE;
        sysmsg.cntlost = 0;

        return;
    }

    if (node == NULL) {
        *len = 1;
        return;
    }

    if (upload_photolast_cnt != 0) {
        *len = 1;
        return;
    }

    if (upload_photolast_cnt == 0) {
        upload_photolast_cnt = PFOTO_SEND_CNT;
    }
    if (node->position == photolast_num) {
        *len = 1;
        return;
    }

    if (node->position != photolast_num) {
        photolast_num = node->position;
    }

    sendlen = 11 + 6;
    buf[9] = type & 0xFF;
    buf[10] = (type >> 8) & 0xFF;

    buf[11] = node->position & 0xFF;
    buf[12] = (node->position >> 8) & 0xFF;

    buf[13] = node->interval & 0xFF;
    buf[14] = (node->interval >> 8) & 0xFF;
    buf[15] = sysmsg.pcslaverrst;
    buf[16] = 0;

    headcarpositoon.interval = 0;



    sum = buf[9];

    for (i = 1; i < sendlen - 9; i++) {
        sum ^= buf[9 + i];
    }

    buf[0] = 0xAA;
    buf[1] = 0xAA;
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = sendlen & 0xFF;
    buf[7] = (sendlen >> 8) & 0xFF;
    buf[8] = sum;

    *len = sendlen;

    sysmsg.pcslaverrst = INVALUE;
    sysmsg.cntlost = 0;
}

void fun_send_slaver_heart_err_cmd(u8* buf, u16* len, u16 type)
{
    u16 sendlen = 0;
    u8 sum;
    u8 i = 0;

    sendlen = 11 + slaverheartmsg.errnum + 1;
    buf[9] = type & 0xFF;
    buf[10] = (type >> 8) & 0xFF;
    buf[11] = slaverheartmsg.errnum;

    for (i = 0; i < slaverheartmsg.errnum; i++) {
        buf[12 + i] = slaverheartmsg.errslaver[i];
    }

    sum = buf[9];

    for (i = 1; i < sendlen - 9; i++) {
        sum ^= buf[9 + i];
    }

    buf[0] = 0xAA;
    buf[1] = 0xAA;
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = sendlen & 0xFF;
    buf[7] = (sendlen >> 8) & 0xFF;
    buf[8] = sum;

    *len = sendlen;

    slaverheartmsg.errnum = 0;
}

void vpcfun_send_slaver_photo_cnt_cmd(u8* buf, u16* len, u16 type)
{
    u8  sum;
    u16 sendlen = 0;
    u16 i;
    sphotobdcnt* node = NULL;

    sendlen = 11 + 8;

    node = vphoto_get_from_slaverbdqueue();

    if (node == NULL) {
        *len = 1;
        return;
    }

    if (node->error != 0) {

        photo_bd_cnt_buff[0] = node->bdindex;
        photo_bd_cnt_buff[1] = (node->bdindex >> 8) & 0xFF;
        photo_bd_cnt_buff[2] = node->photocnt & 0xFF;
        photo_bd_cnt_buff[3] = (node->photocnt >> 8) & 0xFF;
        photo_bd_cnt_buff[4] = node->intervel & 0xFF;
        photo_bd_cnt_buff[5] = (node->intervel >> 8) & 0xFF;
        photo_bd_cnt_buff[6] = node->error & 0xFF;
        photo_bd_cnt_buff[7] = (node->error >> 8) & 0xFF;


        buf[9] = type & 0xFF;
        buf[10] = (type >> 8) & 0xFF;

        for (i = 0; i < sendlen - 11; i++) {
            buf[11 + i] = photo_bd_cnt_buff[i];
        }


        sum = buf[9];
        for (i = 1; i < sendlen - 9; i++)
        {
            sum ^= buf[9 + i];
        }
        buf[0] = 0xAA;
        buf[1] = 0xAA;
        buf[2] = 0x01;
        buf[3] = 0x00;
        buf[4] = 0x00;
        buf[5] = 0x00;
        buf[6] = sendlen & 0xFF;
        buf[7] = (sendlen >> 8) & 0xFF;
        buf[8] = sum;
        *len = sendlen;

        return;
    }

    if (node->bdindex == 1) {
        if (upload_photoone_cnt != 0) {
            *len = 1;
            return;
        }
        if (upload_photoone_cnt == 0) {
            upload_photoone_cnt = PFOTO_SEND_CNT;
        }

        if (node->photocnt == photoone_num) {
            *len = 1;
            return;
        }

        if (node->photocnt != photoone_num) {
            photoone_num = node->photocnt;
        }
    }

    if (node->bdindex == 2) {
        if (upload_phototwo_cnt != 0) {
            *len = 1;
            return;
        }
        if (upload_phototwo_cnt == 0) {
            upload_phototwo_cnt = PFOTO_SEND_CNT;
        }

        if (node->photocnt == phototwo_num) {
            *len = 1;
            return;
        }

        if (node->photocnt != phototwo_num) {
            phototwo_num = node->photocnt;
        }
    }

    photo_bd_cnt_buff[0] = node->bdindex;
    photo_bd_cnt_buff[1] = (node->bdindex >> 8) & 0xFF;
    photo_bd_cnt_buff[2] = node->photocnt & 0xFF;
    photo_bd_cnt_buff[3] = (node->photocnt >> 8) & 0xFF;
    photo_bd_cnt_buff[4] = node->intervel & 0xFF;
    photo_bd_cnt_buff[5] = (node->intervel >> 8) & 0xFF;
    photo_bd_cnt_buff[6] = node->error & 0xFF;
    photo_bd_cnt_buff[7] = (node->error >> 8) & 0xFF;


    buf[9] = type & 0xFF;
    buf[10] = (type >> 8) & 0xFF;

    for (i = 0; i < sendlen - 11; i++) {
        buf[11 + i] = photo_bd_cnt_buff[i];
    }


    sum = buf[9];
    for (i = 1; i < sendlen - 9; i++)
    {
        sum ^= buf[9 + i];
    }
    buf[0] = 0xAA;
    buf[1] = 0xAA;
    buf[2] = 0x01;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = sendlen & 0xFF;
    buf[7] = (sendlen >> 8) & 0xFF;
    buf[8] = sum;
    *len = sendlen;
}

void send_message_to_sever(void)
{
    u16 msg_type;
    //   u8 send_len = 0;
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
    case REPLY_RECV_MSG_SLAVER_PHOTO_CONF_TYPE:
        reply_recv_msg(&(tcp_client_list[0].tcp_send_buf[0]), &(tcp_client_list[0].tcp_send_len), msg_type, reply_index_error);
        tcp_client_list[0].tcp_send_en = 1;
#ifdef  WIFI_TEST
        if( WCS_ACK_timer > 100 )
        {
            send_len = sprintf(str, "TD:%d\n" , WCS_ACK_timer);
            //DEBUG_process(str,send_len);
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
            //DEBUG_process(str,send_len);
        }
#endif
        break;
    case SEND_MSG_CAR2WCS_ONLINE_TYPE:
        send_msg_car2wcs_online_cmd(&(tcp_client_list[0].tcp_send_buf[0]), &(tcp_client_list[0].tcp_send_len), msg_type);
        tcp_client_list[0].tcp_send_en = 1;
        WCS_reply_timer_out = 100;
        break;
    case SEND_MSG_CAR2WCS_CARPOSITION_TYPE:
        fun_head_car_position_cmd(&(tcp_client_list[0].tcp_send_buf[0]), &(tcp_client_list[0].tcp_send_len), msg_type);
        tcp_client_list[0].tcp_send_en = 1;
        break;
    case SEND_MSG_CAR2WCS_SLAVER_HEART_TYPE:
        fun_send_slaver_heart_err_cmd(&(tcp_client_list[0].tcp_send_buf[0]), &(tcp_client_list[0].tcp_send_len), msg_type);
        tcp_client_list[0].tcp_send_en = 1;
        break;
    case SEND_MSG_BD2PC_SLAVER_PHOTO_CNT:
        vpcfun_send_slaver_photo_cnt_cmd(&(tcp_client_list[0].tcp_send_buf[0]), &(tcp_client_list[0].tcp_send_len), msg_type);
        tcp_client_list[0].tcp_send_en = 1;
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
    u8  i = 0, crc = 0; //err,
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

void vfun_slaver_heart_increase(void)
{
    u16 i = 0;

    if (slaverheartmsg.num == 0) return;

    for (i = 0; i < slaverheartmsg.num; i++) {
        slaverheartmsg.heartcnt[i]++;
    }
}

void vfun_upload_slaver_err_heart_program(void)
{
    u16 i = 0;
    u16 j = 0;

    for (i = 0; i < slaverheartmsg.num; i++) {
        if (slaverheartmsg.heartcnt[i] >= 5) {
            slaverheartmsg.heartcnt[i] = 5;
            slaverheartmsg.errslaver[j] = i + 2;
            j++;
        }
    }

    if (j == 0) return;

    slaverheartmsg.errnum = j;

    AddSendMsgToQueue(SEND_MSG_CAR2WCS_SLAVER_HEART_TYPE);


}
