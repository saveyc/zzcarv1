#include "stm32f10x.h"
#include "stm32f107.h"
#include "main.h"
//#ifdef  SLAVE
u8  zongxian_send_buff[ZONGXIAN_BUFF_SIZE / 100];
u8  zongxian_recv_buff[ZONGXIAN_BUFF_SIZE];
//#else
//u8  zongxian_send_buff[ZONGXIAN_BUFF_SIZE];
//u8  zongxian_recv_buff[ZONGXIAN_BUFF_SIZE / 5];
//#endif
u8 Debug_recv_buff[DEBUG_BUFF_SIZE];
u8 Debug_send_buff[DEBUG_BUFF_SIZE];
u8 Debug_recv_count;
u8 DEBUG_USART_tmr = 0;
u16 zongxian_send_count;
u16 zongxian_recv_count;
u8  zongxian_commu_state = RECV_DATA;
u8  zongxian_tmr = 0;
u8 zongxian_send_flag = 0;
u8 zongxian_send_dely = 0;
u8 zongxian_send_agnain_flag = 0;
u16 statue_car_num = 0;
u8  statue_car_count = 0;
u16 statue_guangdian_num = 0;
extern u32 s_speed_zhuxian_u32;
u8  Isphotovalue = 0;
void zongxian_send_process(void)
{
    TX_ZONGXIAN_485;
    if( zongxian_send_count > ZONGXIAN_BUFF_SIZE )
    {
        return;
    }
    DMA_ClearFlag(DMA1_FLAG_TC7);
    USART_DMACmd(ZONGXIAN_USART, USART_DMAReq_Tx, DISABLE);
    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel7, zongxian_send_count);
    USART_DMACmd(ZONGXIAN_USART, USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA1_Channel7, ENABLE);
    zongxian_commu_state = SENDING_DATA;
    zongxian_send_flag++;
    zongxian_send_dely = 0;
}

void zongxian_uart_broadcast(u8 *point, u16 msg_type)
{
    u16 i;

    if( msg_type == GUANGDIAN_CMD_TYPE )
    {
        zongxian_send_buff[0] = '*';
        zongxian_send_buff[1] = INPUT2_count;
        zongxian_send_buff[2] = INPUT2_count >> 8;
        zongxian_send_buff[3] = zongxian_send_buff[0] + zongxian_send_buff[1] + zongxian_send_buff[2];
        zongxian_send_count = 4;

    }
    else if( msg_type == RECV_MSG_WCS2CAR_ONLINE_TYPE )
    {
        zongxian_send_count = (point[6] | point[7] << 8);
        for(i = 0; i < zongxian_send_count; i++)
        {
            zongxian_send_buff[i] =  *(point + i);
        }
        zongxian_send_flag = 0;

    }
    else if(msg_type == RECV_MSG_WCS2CAR_LOAD_OPT_RUNTIME_PARAS)
    {

        zongxian_send_count = (point[6] | point[7] << 8);
        for(i = 0; i < zongxian_send_count; i++)
        {
            zongxian_send_buff[i] =  *(point + i);
        }
        zongxian_send_flag = 0;
    }
    else if(msg_type == RECV_MSG_WCS2CAR_LOAD_CMD_TYPE)
    {
        //*((sWCS2Car_load_CMD_Data*)zongxian_send_buff) = *((sWCS2Car_load_CMD_Data*)point);
        //zongxian_send_count = sizeof(sWCS2Car_load_CMD_Data);
        zongxian_send_count = (point[6] | point[7] << 8);
        for(i = 0; i < zongxian_send_count; i++)
        {
            zongxian_send_buff[i] =  *(point + i);
        }
        zongxian_send_flag = 0;
    }
    else if(msg_type == RECV_MSG_WCS2CAR_FENJIAN_EN_TYPE)
    {
        //*((sWCS2Car_load_CMD_Data*)zongxian_send_buff) = *((sWCS2Car_load_CMD_Data*)point);
        //zongxian_send_count = sizeof(sWCS2Car_load_CMD_Data);
        zongxian_send_count = (point[6] | point[7] << 8);
        for(i = 0; i < zongxian_send_count; i++)
        {
            zongxian_send_buff[i] =  *(point + i);
        }
        zongxian_send_flag = 0;
    }
    else if(msg_type == RECV_MSG_WCS2CAR_UNLOAD_RUN_DATA)
    {
        //*((sWCS2Car_load_CMD_Data*)zongxian_send_buff) = *((sWCS2Car_load_CMD_Data*)point);
        //zongxian_send_count = sizeof(sWCS2Car_load_CMD_Data);
        zongxian_send_count = (point[6] | point[7] << 8);
        for(i = 0; i < zongxian_send_count; i++)
        {
            zongxian_send_buff[i] =  *(point + i);
        }
        zongxian_send_flag = 0;
    }
    else if(msg_type == RECV_MSG_WCS2CAR_PARA_TYPE)
    {
        //*((sWCS2Car_Para_Data*)zongxian_send_buff) = *((sWCS2Car_Para_Data*)point);
        //zongxian_send_count = sizeof(sWCS2Car_Para_Data);
        zongxian_send_count = (point[6] | point[7] << 8);
        for(i = 0; i < zongxian_send_count; i++)
        {
            zongxian_send_buff[i] =  *(point + i);
        }
        zongxian_send_flag = 0;
    }
    else
    {
        return;
    }
    zongxian_send_process();
}



#ifdef SLAVE
/************************************************************

*************************************************************/
void slave_send_moto_process(void)
{
    //u8 send_buff[MOTO_MAX_COUNT + 2+2+1+1];
    u8 i, sum;
    u16 INPUT2_count_error_backup = 0;
    if( moto_statue_comm_en != 0 )
    {
        TX_DEBUG_485;
        Debug_send_buff[0] = 0x55;
        Debug_send_buff[1] = 0x55;
        Debug_send_buff[2] = XIAOCHE_START_NUM;
        Debug_send_buff[3] = XIAOCHE_START_NUM >> 8;
        Debug_send_buff[4] = MOTO_MAX_COUNT;

        INPUT2_count_error_backup = INPUT2_count_error;
        INPUT2_count_error = 0;
        INPUT2_count_error_backup |= XIAOCHE_GROUP_NUM;

        Debug_send_buff[5] = INPUT2_count_error_backup;
        Debug_send_buff[6] = INPUT2_count_error_backup >> 8;
        if( guangdian_error == 1)
        {
            for( i = 0; i < MOTO_MAX_COUNT; i++ )
            {
                moto_para[i].statue |= 0x10;
            }
        }
        else
        {
            for( i = 0; i < MOTO_MAX_COUNT; i++ )
            {
                moto_para[i].statue &= 0xef;
            }
        }
        for(i = 0; i < MOTO_MAX_COUNT; i++)
        {
            Debug_send_buff[7 + i] = moto_para[i].statue;
        }
        sum = 0;
        for(i = 0; i < (MOTO_MAX_COUNT + 2 + 2 + 1 + 2); i++)
        {
            sum += Debug_send_buff[i];
        }
        Debug_send_buff[MOTO_MAX_COUNT + 2 + 2 + 1 + 2] = sum;
        for(i = 0; i < (MOTO_MAX_COUNT + 2 + 2 + 1 + 1 + 2); i++)
        {
            USART_SendData(DEBUG_USART, Debug_send_buff[i]);
            while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
        }
        moto_statue_comm_en = 0;
        RX_DEBUG_485;
    }
}

static u32 rev_cnt_test = 0;
void slave_recv_test(void)
{
    u8 send_len = 0;
    u8 i;
    char str[25];
    send_len = sprintf(str, "recv_count:%d\n" , rev_cnt_test);

    TX_DEBUG_485;
    for(i = 0 ; i < send_len ; i++)
    {
        USART_SendData(DEBUG_USART, str[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
    //DEBUG_process(str,send_len);
    RX_DEBUG_485;
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

u32 s_last_ID_u32 = 0xffffffff;

u32 s_load_unload_packet = 0;

void recv_slaver_photo_config_process(u8* point)
{
    u8 i = 0;
    u8 num = 0;

    Isphotovalue = 0;
    num = point[1];

    for (i = 0; i < num; i++) {
        if (point[2 + i] == key_value) {
            Isphotovalue = 1;
        }
    }
}

void zongxian_uart_recv(void)
{
    MSG_HEAD_DATA *head;
    sLoad_platform_paras *pPlatform_runtime_paras;
    u16  i,  k = 0; //str_addr, len,tmp, j,
    u8 protocol_error_u8 = 0, packet_count_u8 = 0;
    u16 head_size_u16 = 0, speed_size_u16 = 0, load_info_size_u16 = 0, unload_info_size = 0;
    u32 speed_tmp_u32 = 0;
    //if(zongxian_commu_state == RECV_DATA_END)
    //{
    //zongxian_commu_state = RECV_DATA;
#ifdef  ZONGXIAN_TEST_MODE
    TX_DEBUG_485
    for(i = 0 ; i < zongxian_recv_count ; i++)
    {
        USART_SendData(DEBUG_USART, zongxian_recv_buff[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
    RX_DEBUG_485;
#endif
    if(recv_msg_check(zongxian_recv_buff, zongxian_recv_count))
    {
        head = (MSG_HEAD_DATA *)zongxian_recv_buff;
        switch(head->MSG_TYPE)
        {

        case RECV_MSG_WCS2CAR_ONLINE_TYPE :
            INPUT2_count_error = 0;
            motor_position_reset();//19/6/25 add
            
            break;

        case RECV_MSG_WCS2CAR_UNLOAD_RUN_DATA:
            sWCS2CarUnloadRunData = *((sWCS2Car_Unload_run_Data *)zongxian_recv_buff);
            max_guandian_count_u16 = sWCS2CarUnloadRunData.car_num_u16;

            if( (max_guandian_count_u16 - XIAOCHE_START_NUM ) > 16 )
            {
                MOTO_MAX_COUNT = 16;
            }
            else
            {
                MOTO_MAX_COUNT = max_guandian_count_u16 - XIAOCHE_START_NUM + 1;
            }
            break;
        case RECV_MSG_WCS2CAR_FENJIAN_EN_TYPE:
            wcs2carSysEnData = *((sWCS2Car_SYS_EN_Data *)zongxian_recv_buff);
            check_motor_flag = 0;
            INPUT2_count_error = 0;
            guangdian_error = 0;
            s_LastPhotoCount = 0xffffffff;
            sysmsg.cntlost = 0;
            break;
        case RECV_MSG_SLAVER_PHOTO_CONF_TYPE:
            recv_slaver_photo_config_process(zongxian_recv_buff + 11);;
            break;
        case RECV_MSG_WCS2CAR_LOAD_OPT_RUNTIME_PARAS :
            /*
                   -------------------------------------------------------------------------------------------------
                   |            Frame Head (11B)          | adjust_count |   load_runtime_paras (adjust_count*7)(B)|
                   |--------------------------------------|--------------|-----------------------------------------|
                   |0xaaaa|seq(4B)|len(2B)|crc(1B)|cmd(2B)|      1B      |    sLoad_platform_paras[adjust_count]   |
                   -------------------------------------------------------------------------------------------------
            */
            packet_count_u8 = *(zongxian_recv_buff + 11);

            if( (packet_count_u8 <= LOAD_NUM) && (packet_count_u8 > 0) )
            {
                for(i = 0; i < packet_count_u8; i++)
                {
                    pPlatform_runtime_paras = (sLoad_platform_paras *) (zongxian_recv_buff + 12 + i * sizeof(sLoad_platform_paras));
                    if(pPlatform_runtime_paras->platform_index_u8 < LOAD_NUM)
                    {
                        wcs2CarParaData.a_u16[pPlatform_runtime_paras->platform_index_u8] = pPlatform_runtime_paras->a_u16;
                        wcs2CarParaData.b_u16[pPlatform_runtime_paras->platform_index_u8] = pPlatform_runtime_paras->b_u16;
                        wcs2CarParaData.c_s16[pPlatform_runtime_paras->platform_index_u8] = pPlatform_runtime_paras->c_s16;
                    }//if(pPlatform_runtime_paras->platform_index_u8 < LOAD_NUM)
                }//for(i = 0; i < packet_count_u8; i++)
            }// if( (packet_count_u8 <= LOAD_NUM) && (packet_count_u8 > 0) )
            break;

        case RECV_MSG_WCS2CAR_LOAD_CMD_TYPE://四包合一
            if(head->MSG_ID != s_last_ID_u32)
            {
                if(s_last_ID_u32 != 0xffffffff)
                {
                    if(head->MSG_ID != s_last_ID_u32+1)//数据帧计数不连续表示丢四包合一
                    {
                        INPUT2_count_error |= 0x400;
                    }
                }
                s_last_ID_u32 = head->MSG_ID;
                protocol_error_u8 = 0;
                head_size_u16 = sizeof(MSG_HEAD_DATA);

                speed_tmp_u32 = *(u32 *)(zongxian_recv_buff + head_size_u16);
                s_speed_zhuxian_u32 = speed_tmp_u32 * 10;  //转换成 mm/s
                speed_size_u16 = 4;

                Load_info_t.platform_num_u8 = *(u8 *)(zongxian_recv_buff + head_size_u16 + speed_size_u16);
                if(Load_info_t.platform_num_u8 <= MAX_LOAD_PLATFORM_NUM)
                {
                    memcpy(&(Load_info_t.car_load_info_t), (u8 *)(zongxian_recv_buff + head_size_u16 + speed_size_u16 + 1), Load_info_t.platform_num_u8 * sizeof(sCar_Load_Info));
                }
                else
                {
                    protocol_error_u8 = 1;
                }
                load_info_size_u16 = 1 + Load_info_t.platform_num_u8 * sizeof(sCar_Load_Info);

                Unload_info_t.unload_car_num_u8 = *(u8 *)(zongxian_recv_buff + head_size_u16 + speed_size_u16 + load_info_size_u16);
                if(Unload_info_t.unload_car_num_u8 <= MAX_UNLOAD_CAR_NUM)
                {
                    memcpy(&(Unload_info_t.car_unload_info_t), (u8 *)(zongxian_recv_buff + 1 + head_size_u16 + speed_size_u16 + load_info_size_u16), Unload_info_t.unload_car_num_u8 * sizeof(sCar_unload_Data));
                }
                else
                {
                    protocol_error_u8 = 1;
                }
                unload_info_size = 1 + Unload_info_t.unload_car_num_u8 * sizeof(sCar_unload_Data);

                for(k = 0; k < EXIT_NUM; k++)
                    wcs2CarParaData.exit_stop[k] = 0;

                if( (!protocol_error_u8) && (head->MSG_LENGTH > ( head_size_u16 + speed_size_u16 + load_info_size_u16 + unload_info_size)) )
                {
                    xiaoliao_IO_exit_process(zongxian_recv_buff + head_size_u16 + speed_size_u16 + load_info_size_u16 + unload_info_size );
                }

                rev_cnt_test++;
                get_xialiao_num_process();
                get_load_num();
            }
            break;
        case RECV_MSG_WCS2CAR_PARA_TYPE:
            // wcs2CarParaData = *((sWCS2Car_Para_Data*)zongxian_recv_buff);
            xiaoliao_exit_process(zongxian_recv_buff + 11);

            break;
        case RECV_MSG_WCS2CAR_CHECK_CAR_TYPE: //测试小车命令 0x1133
            check_car_cmd_process(zongxian_recv_buff);
            break;
        default:
            break;
        }

    }
    //rev_cnt_test++;
    zongxian_recv_count = 0;
    //}
}
#endif
/*************************************************

*************************************************/
void recv_moto_statue_process(void)
{
    RX_DEBUG_485;
    Debug_recv_count = 0;
}
