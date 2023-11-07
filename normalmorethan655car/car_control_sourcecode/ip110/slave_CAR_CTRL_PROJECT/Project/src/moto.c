#include "stm32f10x.h"
#include "stm32f107.h"
#include "main.h"
uint8_t     XIAOCHE_GROUP_NUM;
uint16_t    XIAOCHE_START_NUM;
uint8_t     MOTO_MAX_COUNT;
uint8_t moto_send_buff[20];
uint8_t moto_recv_buff[20];
uint8_t cur_moto_send_count, moto_send_count, moto_recv_count;
uint8_t	moto_commu_statue;
uint8_t moto_recv_dely = 0;
moto_para_struct   moto_para[MOTO_MAX_COUNT1];
uint8_t  moto_send_num = 0;
uint8_t moto_recv_statue;

uint8_t moto_start_ctr = 0;
uint8_t cur_moto_para_num = 0;
uint8_t pre_moto_para_num = 0;
uint8_t moto_ctr_statue = WAIT_RECV_RUN;
uint8_t	moto_reply_outtimer = 0;
uint8_t moto_reply_count = 0;

uint8_t   moto_test_index = 0;
u8 moto_message_index = 0;
u8 LOAD_test_dely = 0;

extern u32 s_speed_zhuxian_u32;
/**************************************************

**************************************************/
void  uart_send_dma_process(void)
{
    if( moto_commu_statue == SENDING_DATA )
    {
        if( DMA_GetFlagStatus(DMA1_FLAG_TC7) )
        {
            moto_ready_recv_process();
        }
    }
}

/****************************************

*****************************************/
void moto_ready_recv_process(void)
{
    moto_commu_statue = RECV_DATA;
    RX_MOTO_485;
    moto_recv_count = 0;
    moto_recv_statue = 0;
}
/************************************

*************************************/
void moto_send_process(void)
{
    TX_MOTO_485;

    if( moto_commu_statue == SEND_DATA )
    {
        moto_commu_statue = SENDING_DATA;

        DMA_Cmd(DMA1_Channel4, DISABLE);
        DMA1_Channel4->CNDTR = moto_send_count + 2; // 设置要发送的字节数目
        DMA_Cmd(DMA1_Channel4, ENABLE);        //开始DMA发送

    }
}

/************************************************

************************************************/
void moto_recv_now_process(void)
{
    if(moto_recv_buff[0] == 0x99 )//moto_recv_statue == 1  //
    {
        moto_recv_statue = 1;
        if( moto_recv_count >= 4 )
        {
            if( moto_recv_buff[1] == moto_para[cur_moto_para_num].num )
            {
                if( moto_recv_buff[3] == ( moto_recv_buff[1] ^ moto_recv_buff[2] ) )
                {
                    moto_para[cur_moto_para_num].statue &= 0xF0;
                    moto_para[cur_moto_para_num].statue |= moto_recv_buff[2];
                    if( moto_ctr_statue == WAIT_RECV_PARA )
                    {
                        if(check_motor_flag == 1)
                        {
                            moto_para[cur_moto_para_num].run_en = MOTO_OPT_RUN_EN;
                            check_motor_flag = 0;
                        }
                        moto_para[cur_moto_para_num].statue &= 0x7F;
                        cur_moto_para_num++;
                        pre_moto_para_num = cur_moto_para_num;
                        moto_ctr_statue = SEND_PARA_STATUE;
                        moto_reply_count = 0;
                        moto_reply_outtimer = 0;
                    }
                }
            }
        }
    }
    else
    {
        if( moto_recv_buff[moto_recv_count - 1] == 0x99 )
        {
            moto_recv_statue = 1;
            moto_recv_buff[0] = 0x99;
            moto_recv_count = 1;
        }
    }
}

/******************************************************8

*******************************************************/
void moto_message_process(void)
{
    uint8_t i;
    u8 send_len = 0;
    char str[25];
#ifdef SLAVE
    TX_DEBUG_485;
#endif
    //for( i = 0; i < MOTO_MAX_COUNT;i++)
    //{
    send_len = sprintf(str, "N:%d " , moto_message_index + 1);
#ifdef SLAVE
    for(i = 0 ; i < send_len ; i++)
    {
        USART_SendData(DEBUG_USART, str[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
#else
    DEBUG_process(str, send_len);
#endif
    send_len = sprintf(str, "X:%d " , moto_para[moto_message_index].xialiao_postion);
#ifdef SLAVE
    for(i = 0 ; i < send_len ; i++)
    {
        USART_SendData(DEBUG_USART, str[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
#else
    DEBUG_process(str, send_len);
#endif
    send_len = sprintf(str, "L:%d " , moto_para[moto_message_index].load_postion);
#ifdef SLAVE
    for(i = 0 ; i < send_len ; i++)
    {
        USART_SendData(DEBUG_USART, str[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
#else
    DEBUG_process(str, send_len);
#endif
    send_len = sprintf(str, "P:%d " , moto_para[moto_message_index].cur_postion);
#ifdef SLAVE
    for(i = 0 ; i < send_len ; i++)
    {
        USART_SendData(DEBUG_USART, str[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
#else
    DEBUG_process(str, send_len);
#endif
    send_len = sprintf(str, "Z:%d\n" , moto_para[moto_message_index].statue);
#ifdef SLAVE
    for(i = 0 ; i < send_len ; i++)
    {
        USART_SendData(DEBUG_USART, str[i]);
        while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);
    }
#else
    DEBUG_process(str, send_len);
#endif
    //}
#ifdef SLAVE
    RX_DEBUG_485;
#endif
    if( moto_message_index < ( MOTO_MAX_COUNT - 1 ))
    {
        moto_message_index++;
    }
    else
    {
        moto_message_index = 0;
    }
}

/***************************************************

****************************************************/
#ifdef _UDP_DEBUG_
extern u8 debug_info[500];
extern void  DEBUG_process_udp(u8 *p_data, u16 len);
#endif
void  get_xialiao_num_process(void)
{
    uint16_t i, j, tmp;
    uint16_t m;
    
    for( i = 0; i < Unload_info_t.unload_car_num_u8; i++ )
    {
        tmp = Unload_info_t.car_unload_info_t[i].car_unload_index_u16;

        for(j = 0; j < MOTO_MAX_COUNT; j++)
        {
            if( moto_para[j].xiaoche_num == tmp )//匹配小车编号
            {
                if(Unload_info_t.car_unload_info_t[i].car_unload_exitno_u16 == 0)
                {
                    //下料位置为0表示IOB,直接转动皮带(顺丰IOB功能)
                    moto_para[j].xialiao_postion = 0;
                    moto_para[j].fangxiang = Unload_info_t.car_unload_info_t[i].car_unload_direction_u8;
                    moto_para[j].juli = Unload_info_t.car_unload_info_t[i].car_rotate_length_u16;
                    moto_para[j].speed = 120;
                    moto_para[j].dely = 0;
                    moto_para[j].run_en = MOTO_OPT_SEND_PARA;
                    moto_start_ctr = 1;
                    
                    break;
                }
                if (Unload_info_t.car_unload_info_t[i].car_rotate_length_u16 == 0) {
                    moto_para[j].xialiao_postion = 0xffffffff;
                    moto_para[j].load_postion = 0xFFFF;
                }
                
                for(m = 0; m < wcs2CarParaData.exit_num; m++)
                {
                    if( ( Unload_info_t.car_unload_info_t[i].car_unload_exitno_u16 == wcs2CarParaData.exit_position[m] )  &&
                            ( Unload_info_t.car_unload_info_t[i].car_unload_direction_u8 == wcs2CarParaData.exit_direction[m] )  &&
                            ( wcs2CarParaData.exit_stop[m] == 0 ) )
                    {
                        //更新小车下料动作参数
                        moto_para[j].xialiao_postion = Unload_info_t.car_unload_info_t[i].car_unload_exitno_u16;
                        moto_para[j].load_postion = 0xFFFF;
                        moto_para[j].fangxiang = Unload_info_t.car_unload_info_t[i].car_unload_direction_u8;
                        moto_para[j].juli = Unload_info_t.car_unload_info_t[i].car_rotate_length_u16;
                        moto_para[j].speed = wcs2CarParaData.exit_speed[m];
                        moto_para[j].dely = Unload_info_t.car_unload_info_t[i].car_unload_delay_s16;
                        
                        break;
                    }
                }
                
                break;
            }
        }
    }
}

/************************************************************************

***********************************************************************/
void  get_load_num(void)
{
    u8 platform_index = 0;
    u32 true_or_false = 0;
    uint16_t i, j, tmp, adjust_length = 0;
    s16 temp_s16 = 0;
#ifdef     LOAD_TEST
    u8 send_len = 0;
    char str[25];
#endif
    adjust_length = (uint16_t)(CAR_LENGTH * MAX_LOAD_RUN_LENGTH_RATIO);
    for(i = 0; i < Load_info_t.platform_num_u8; i++)
    {
        if( Load_info_t.car_load_info_t[i].car_load_length_u16 != 0 )
        {
            tmp = Load_info_t.car_load_info_t[i].car_load_index_u16;
            for(j = 0; j < MOTO_MAX_COUNT; j++)
            {
                if( moto_para[j].xiaoche_num == tmp )
                {
                    //匹配小车编号
                    moto_para[j].xialiao_postion = 0xFFFF;
                    moto_para[j].load_postion = wcs2CarParaData.load_position[Load_info_t.car_load_info_t[i].platform_index_u8];
                    platform_index = Load_info_t.car_load_info_t[i].platform_index_u8;
                    Load_info_t.car_load_info_t[i].car_load_length_u16 = wcs2CarParaData.a_u16[platform_index] * Load_info_t.car_load_info_t[i].car_load_length_u16 / 1000 + wcs2CarParaData.b_u16[platform_index];//+=20;
                    if( Load_info_t.car_load_info_t[i].car_load_length_u16 < adjust_length)
                    {
                        moto_para[j].juli = ( Load_info_t.car_load_info_t[i].car_load_length_u16 + ( CAR_LENGTH - Load_info_t.car_load_info_t[i].car_load_length_u16) / 2) / 15;
                        if( moto_para[j].juli > ( adjust_length / 15 ) )//
                        {
                            moto_para[j].juli = adjust_length / 15;
                        }
                    }
                    else
                    {
                        moto_para[j].juli = adjust_length / 15;
                    }

                    temp_s16 = ((((u32)Load_info_t.car_load_info_t[i].car_load_offset_u16 * 2 * 1000) / s_speed_zhuxian_u32)) + wcs2CarParaData.c_s16[platform_index] + Load_info_t.car_load_info_t[i].car_load_delay_s16;
                    temp_s16 = temp_s16 < 0 ? 0 : temp_s16;
                    moto_para[j].dely = temp_s16;
                    
                    if(sWCS2CarUnloadRunData.mainline_speed == 2500)
                        moto_para[j].speed = 120;
                    else
                        moto_para[j].speed = 95;

                    moto_para[j].fangxiang = wcs2CarParaData.load_direction[platform_index];
                    
                    true_or_false =  ((((int)moto_para[j].load_postion) - ((int)moto_para[j].cur_postion)) + (int)max_guandian_count_u16) % max_guandian_count_u16;
                    if( (true_or_false == 1) )
                    {

                        moto_para[j].dely = (moto_para[j].dely) / 10;//180504 去掉一个车的延时
                        moto_para[j].run_en = MOTO_OPT_SEND_PARA;
                        moto_start_ctr = 1;
                    }
                    /*-----------------------------------------------------------------------added by gwf 2016.12.15  end----------------*/

                    break;
                }
            }
        }
    }
}
/****************************************************

*****************************************************/
void write_moto_process(void)
{
    uint8_t i;
#ifndef MOTO_TEST_MODE
    for(i = 0; i < MOTO_MAX_COUNT; i++ )
    {
        moto_para[i].num = i + 1;
        moto_para[i].run_en = MOTO_OPT_NO_CMD;
        moto_para[i].fangxiang = 0;
        moto_para[i].speed = 120;
        moto_para[i].dely = 0;
        moto_para[i].juli = 80;
        moto_para[i].statue = 0;
        moto_para[i].xiaoche_num = XIAOCHE_START_NUM + i;
        moto_para[i].xialiao_postion = 0xffffffff;
        moto_para[i].load_postion = 0xffff;
    }
#else
    moto_para[4 * moto_test_index].num = 1;
    moto_para[4 * moto_test_index].fangxiang = 0;
    moto_para[4 * moto_test_index].speed = 120;
    moto_para[4 * moto_test_index].dely = 0;
    moto_para[4 * moto_test_index].juli = 50;
    moto_para[4 * moto_test_index].run_en = MOTO_OPT_SEND_PARA;
    moto_para[4 * moto_test_index].xiaoche_num = 1;
    moto_para[4 * moto_test_index].xialiao_postion = 3;
    /*moto_para[4*moto_test_index].num = 4*moto_test_index+1;
    moto_para[4*moto_test_index].fangxiang = 0;
    moto_para[4*moto_test_index].speed = 120;
    moto_para[4*moto_test_index].dely = 0;
    moto_para[4*moto_test_index].juli = 50;
    moto_para[4*moto_test_index].run_en = 1;
    moto_para[4*moto_test_index].xiaoche_num= 1;
    moto_para[4*moto_test_index].xialiao_postion = 3;

    moto_para[4*moto_test_index+1].num = 4*moto_test_index+2;
    moto_para[4*moto_test_index+1].fangxiang = 0;
    moto_para[4*moto_test_index+1].speed = 120;
    moto_para[4*moto_test_index+1].dely = 0;
    moto_para[4*moto_test_index+1].juli = 50;
    moto_para[4*moto_test_index+1].run_en = 1;
    moto_para[4*moto_test_index+1].xiaoche_num= 2;
    moto_para[4*moto_test_index+1].xialiao_postion = 5;

    moto_para[4*moto_test_index+2].num = 4*moto_test_index+3;
    moto_para[4*moto_test_index+2].fangxiang = 0;
    moto_para[4*moto_test_index+2].speed = 120;
    moto_para[4*moto_test_index+2].dely = 0;
    moto_para[4*moto_test_index+2].juli = 40;
    moto_para[4*moto_test_index+2].run_en = 1;
    moto_para[4*moto_test_index+2].xiaoche_num= 3;
    moto_para[4*moto_test_index+2].xialiao_postion = 7;

    moto_para[4*moto_test_index+3].num = 4*moto_test_index+4;
    moto_para[4*moto_test_index+3].fangxiang = 0;
    moto_para[4*moto_test_index+3].speed = 120;
    moto_para[4*moto_test_index+3].dely = 0;
    moto_para[4*moto_test_index+3].juli = 70;
    moto_para[4*moto_test_index+3].run_en = 1;
    moto_para[4*moto_test_index+3].xiaoche_num= 4;
    moto_para[4*moto_test_index+3].xialiao_postion = 9;

    if( moto_test_index < 3 )
    {
        moto_test_index++;
    }
    else
    {
        moto_test_index = 0;
    }*/
#endif
}

void motor_position_reset(void)
{
    u8 i;
    
    for(i = 0; i < MOTO_MAX_COUNT; i++ )
    {
        moto_para[i].xialiao_postion = 0xffffffff;
        moto_para[i].load_postion = 0xffff;
    }
}

/******************************************************

********************************************************/
#ifdef _UDP_DEBUG_
extern u8 debug_info[500];
extern void  DEBUG_process_udp(u8 *p_data, u16 len);
#endif
void xialiao_process(void)
{
    u32 true_or_false = 0;
    u32 tmp, cur_postion_tmp;
    u8  i;
    
    pre_INPUT2_count = INPUT2_count;
    for( i = 0; i < MOTO_MAX_COUNT; i++ )
    {
        //计算每个小车的当前位置
        tmp = i * POSTION_JIANGE;
        if(  pre_INPUT2_count > tmp )
        {
            moto_para[i].cur_postion = pre_INPUT2_count - tmp;
        }
        else
        {
            moto_para[i].cur_postion = max_guandian_count_u16 - (tmp - pre_INPUT2_count);
        }
    }

    for( i = 0; i < MOTO_MAX_COUNT; i++ )
    {
        if(moto_para[i].run_en == MOTO_OPT_SEND_PARA)
        {
            moto_para[i].run_en = MOTO_OPT_RUN_EN;
            moto_start_ctr = 1;
        }
    }
    for( i = 0; i < MOTO_MAX_COUNT; i++ )
    {
        true_or_false =  ((((int)moto_para[i].load_postion) - ((int)moto_para[i].cur_postion)) + (int)max_guandian_count_u16) % max_guandian_count_u16;

        if( (true_or_false == 1)  && (moto_para[i].run_en == MOTO_OPT_NO_CMD) && (moto_para[i].load_postion != 0xffff)) // if( moto_para[i].load_postion == cur_postion_tmp )
        {
            moto_para[i].dely = (moto_para[i].dely) / 10;//180504 去掉一个车的延时
            moto_para[i].dely = moto_para[i].dely > 2000 ? 2000 : moto_para[i].dely;
            moto_para[i].run_en = MOTO_OPT_SEND_PARA;
            moto_start_ctr = 1;
        }
    }
    
    for( i = 0; i < MOTO_MAX_COUNT; i++ )
    {
        if(sWCS2CarUnloadRunData.mainline_speed == 2500)
            cur_postion_tmp = (moto_para[i].cur_postion * 100) + 300;//提前三个车判断
        else
            cur_postion_tmp = (moto_para[i].cur_postion * 100) + 200;//提前两个车判断
        
        if( cur_postion_tmp > (max_guandian_count_u16 * 100) )
        {
            cur_postion_tmp = cur_postion_tmp - max_guandian_count_u16 * 100;
        }
        
        if( moto_para[i].xialiao_postion == cur_postion_tmp )
        {
            moto_para[i].dely = moto_para[i].dely  / 10;
            moto_para[i].run_en = MOTO_OPT_SEND_PARA;
            moto_start_ctr = 1;
        }
        else if( ( moto_para[i].xialiao_postion > cur_postion_tmp) && ( ( moto_para[i].xialiao_postion - cur_postion_tmp ) < 100 ) )
        {
            moto_para[i].dely += ((u32)(moto_para[i].xialiao_postion - cur_postion_tmp) * 10 * 1000) / s_speed_zhuxian_u32;
            
            moto_para[i].dely = moto_para[i].dely  / 10;
            moto_para[i].run_en = MOTO_OPT_SEND_PARA;
            moto_start_ctr = 1;
        }
    }
}
/****************************************************

*****************************************************/
void  send_moto_para_process(uint8_t moto_num)
{
    uint8_t buff_tmp[8];
    uint8_t i;

    for(i = 0; i < 8; i++)
    {
        buff_tmp[i] = 0;
    }
    buff_tmp[0] = 0x85;
    buff_tmp[1] = moto_para[moto_num].num;
    //if( moto_para[moto_num].fangxiang != 0 )
    if( moto_para[moto_num].fangxiang == 0 )
    {
        buff_tmp[1] |= 0x40;
    }
    buff_tmp[2] = moto_para[moto_num].speed;
    if( moto_para[moto_num].dely > 127)
    {
        buff_tmp[3] = moto_para[moto_num].dely & 0x7f;
        buff_tmp[5] |= 0x01;
    }
    else
    {
        buff_tmp[3] = moto_para[moto_num].dely;
    }
    buff_tmp[4] = moto_para[moto_num].juli;
    buff_tmp[6] = moto_send_num++;
    if( buff_tmp[6] >= 0x80 )
    {
        buff_tmp[6] = 0;
        moto_send_num = 0;
    }
    for(i = 1; i < 7; i++)
    {
        buff_tmp[7] ^= buff_tmp[i];
    }
    for(i = 0; i < 8; i++)
    {
        moto_send_buff[i] = buff_tmp[i];
    }
    moto_send_count = 8;
    moto_commu_statue = SEND_DATA;
    moto_send_process();
    moto_reply_outtimer = 20;
}

/****************************************************

*****************************************************/
void  send_moto_run_process(void)
{
    uint8_t buff_tmp[8];
    uint8_t i;

    for(i = 0; i < 8; i++)
    {
        buff_tmp[i] = 0;
    }
    buff_tmp[0] = 0x8A;
    if( MOTO_MAX_COUNT >= 7)
    {
        for(i = 0; i < 7; i++ )
        {
            if( moto_para[i].run_en == MOTO_OPT_RUN_EN )
            {
                buff_tmp[1] |= (1 << i);
                moto_para[i].run_en = MOTO_OPT_NO_CMD;
                moto_para[i].xialiao_postion = 0xffffffff;
                moto_para[i].load_postion = 0xffff;
            }
        }
    }
    else
    {
        for(i = 0; i < MOTO_MAX_COUNT; i++ )
        {
            if( moto_para[i].run_en == MOTO_OPT_RUN_EN )
            {
                buff_tmp[1] |= (1 << i);
                moto_para[i].run_en = MOTO_OPT_NO_CMD;
                moto_para[i].xialiao_postion = 0xffffffff;
                moto_para[i].load_postion = 0xffff;
            }
        }
    }
    if( MOTO_MAX_COUNT >= 7)
    {
        if( moto_para[7].run_en == MOTO_OPT_RUN_EN )
        {
            buff_tmp[5] |= 0x01;
            moto_para[i].run_en = MOTO_OPT_NO_CMD;
            moto_para[i].xialiao_postion = 0xffffffff;
            moto_para[i].load_postion = 0xffff;
        }
    }
    if( MOTO_MAX_COUNT > 8)
    {
        for(i = 8; i < 15; i++ )
        {
            if( moto_para[i].run_en == MOTO_OPT_RUN_EN )
            {
                buff_tmp[2] |= (1 << (i - 8));
                moto_para[i].run_en = MOTO_OPT_NO_CMD;
                moto_para[i].xialiao_postion = 0xffffffff;
                moto_para[i].load_postion = 0xffff;
            }
        }
    }
    if( MOTO_MAX_COUNT >= 16 )
    {
        if( moto_para[15].run_en == MOTO_OPT_RUN_EN )
        {
            buff_tmp[5] |= 0x02;
            moto_para[15].run_en = MOTO_OPT_NO_CMD;
            moto_para[15].xialiao_postion = 0xffffffff;
            moto_para[15].load_postion = 0xffff;
        }
    }
    buff_tmp[6] = moto_send_num++;
    if( buff_tmp[6] >= 0x80 )
    {
        buff_tmp[6] = 0;
        moto_send_num = 0;
    }
    for(i = 1; i < 7; i++)
    {
        buff_tmp[7] ^= buff_tmp[i];
    }
    for(i = 0; i < 8; i++)
    {
        moto_send_buff[i] = buff_tmp[i];
    }
    moto_send_count = 8;
    moto_commu_statue = SEND_DATA;
    moto_send_process();
}

/*********************************************************8

**********************************************************/
void moto_ctr_process(void)
{
    switch(moto_ctr_statue)
    {
    case SEND_PARA_STATUE:
        for(cur_moto_para_num = pre_moto_para_num; cur_moto_para_num < MOTO_MAX_COUNT; cur_moto_para_num++)
        {
            if( moto_para[cur_moto_para_num].run_en == MOTO_OPT_SEND_PARA )
            {
                send_moto_para_process(cur_moto_para_num);
                moto_ctr_statue = WAIT_RECV_PARA;
                //moto_ctr_statue = WAIT_RECV_RUN;
                //pre_moto_para_num = cur_moto_para_num;
                return;
            }
        }
        send_moto_run_process();
        moto_ctr_statue = WAIT_RECV_RUN;
        break;
    case WAIT_RECV_PARA:
        if( moto_reply_outtimer == 0 )
        {
            if( moto_reply_count == 0 )
            {
                moto_reply_count++;
                //send_moto_para_process(cur_moto_para_num);
            }
            else
            {
                moto_para[cur_moto_para_num].statue |= 0x80;
                cur_moto_para_num++;
                pre_moto_para_num = cur_moto_para_num;
                moto_reply_count = 0;
            }
            moto_ctr_statue = SEND_PARA_STATUE;
        }
        break;
    case WAIT_RECV_RUN:
        pre_moto_para_num = 0;
        if( moto_start_ctr != 0 )
        {
            moto_ctr_statue = SEND_PARA_STATUE;
            moto_start_ctr = 0;
            moto_reply_count = 0;
#ifdef  MOTO_TEST_MODE
            write_moto_process();
#endif
        }
        break;
    default:
        break;
    }
}


