#include "stm32f10x.h"
#include "stm32f107.h"
#include "main.h"
//#ifdef  SLAVE
//u8  zongxian_send_buff[ZONGXIAN_BUFF_SIZE / 5];
//u8  zongxian_recv_buff[ZONGXIAN_BUFF_SIZE];
//#else
u8  zongxian_send_buff[ZONGXIAN_BUFF_SIZE];
u8  zongxian_recv_buff[ZONGXIAN_BUFF_SIZE / 100];
//#endif
u8 Debug_recv_buff[DEBUG_BUFF_SIZE];
u8 Debug_recv_buff_shadow[DEBUG_BUFF_SIZE];
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

    if(RECV_MSG_WCS2CAR_LOAD_CMD_TYPE == ((zongxian_send_buff[10] << 8) | zongxian_send_buff[9]))
        zongxian_send_flag++;//四包合一数据发送两次
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

    }  //
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
/*************************************************

*************************************************/
void recv_moto_statue_process(void)
{
    u8 i, sum;
    u16 car_num;
    //DEBUG_process(Debug_recv_buff,Debug_recv_count);
    if( (Debug_recv_buff_shadow[0] == 0x55) && (Debug_recv_buff_shadow[1] == 0x55) && (Debug_recv_count > 6 ))
    {
        //#ifdef  WIFI_TEST
        //DEBUG_process(Debug_recv_buff,Debug_recv_count);
        //#endif
        if( Debug_recv_count == ( Debug_recv_buff_shadow[4] + 2 + 2 + 1 + 1 + 2) )
        {
            sum = 0;
            for(i = 0; i < (Debug_recv_count - 1); i++)
            {
                sum += Debug_recv_buff_shadow[i];
            }
            if( sum == Debug_recv_buff_shadow[i] )
            {

                car_num = Debug_recv_buff_shadow[3];
                car_num <<= 8;
                car_num |= Debug_recv_buff_shadow[2];
                for( i = 0; i < Debug_recv_buff_shadow[4]; i++ )
                {
                    sys_moto_statue[car_num - 1 + i] = Debug_recv_buff_shadow[7 + i];
                }
                statue_car_num = car_num;
                statue_car_count = Debug_recv_buff_shadow[4];
                moto_statue_comm_en = 1;
                statue_guangdian_num = Debug_recv_buff_shadow[6];
                statue_guangdian_num <<= 8;
                statue_guangdian_num |= Debug_recv_buff_shadow[5];
            }
        }
    }
    RX_DEBUG_485;
    Debug_recv_count = 0;
}
