/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32_eth.h"
#include "netconf.h"
#include "main.h"
#include "TCPclient.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  1

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;
u16 sec_reg = 1000;
u16 ms500_reg = 500;
u8 broadcast_pthoto_over_flag = 0;
u8 slave_send_debug_flag = 0;
u16 debug_message_dely = 200;
u16 ms500_send_flag = 0;
u16  sec_flag = 0;
u8 DEBUG_recv_flag = 0;
u8 key_value = 0;
u16 max_guandian_count_u16 = 0;
u8 check_motor_flag = 0;
u8 can_send_cnt = 0;
u8 can_send_cnt_bk = 0;

u8 opt_inc_flag = 0;
u8 opt_inc_cnt = 0;
u8 opt_rst_flag = 0;
u8 opt_rst_cnt = 0;
u16 s10_cnt = 0;

/* Private function prototypes -----------------------------------------------*/
void System_Periodic_Handle(void);
extern u8 zongxian_send_agnain_flag;
void main_onems_process(void);
u16 onems_flag = INVALUE;
//1s
void sec_process(void)
{
    u8 i;
    sCar2Wcs_headcar_positon node;
    sphotobdcnt slnode;

    if( sec_flag == 1 )
    {
        sec_flag = 0;
        s10_cnt++;

        if (sysmsg.cntlost != 0) {
            node.position = INPUT2_count;
            node.interval = 0;
            vphoto_add_to_slaver_position_queue(node);
        }

        if (phototrig.cntlost != 0) {
            slnode.bdindex = 1;
            slnode.photocnt = phototrig.curposition;
            slnode.intervel = phototrig.interval;
            slnode.error = phototrig.cntlost;
            vphoto_add_to_slaver_bd_queue(slnode);
            phototrig.cntlost = 0;
        }


        if (gndtrig.cntlost != 0) {
            slnode.bdindex = 0xAA;
            slnode.photocnt = gndtrig.curposition;
            slnode.intervel = gndtrig.interval;
            slnode.error = gndtrig.cntlost;
            vphoto_add_to_slaver_bd_queue(slnode);
            gndtrig.cntlost = 0;
        }

        vfun_slaver_heart_increase();
        if (s10_cnt >= 10) {
            s10_cnt = 0;
            vfun_upload_slaver_err_heart_program();
        }
        
        if( heart_dely != 0 )
        {
            heart_dely--;
        }
        
        if(LED_STATUE)
        {
            LED_ON;
        }
        else
        {
            LED_OFF;
        }
        fun_upload_heart_err_process();
//        fun_upload_heart_unconnect_process();
        //moto_statue_comm_en = 1;
#ifdef  MOTO_TEST_MODE
        moto_start_ctr = 1;
#endif
        for(i = 0; i < TCP_CLIENT_MAX_NUM; i++)
        {
            if(tcp_client_list[i].connect_dely != 0 )
            {
                tcp_client_list[i].connect_dely--;
            }
        }
#ifdef SLAVE_TEST_ZONGXIAN
        moto_message_process();
        slave_recv_test();
#endif
    }
}
/***********************************************************************

****************************************************************************/
void scan_key_process(void)
{
    u8 tmp, i;

    tmp = 0;
    for(i = 0; i < 20; i++)
    {
        if( key1_STATUE )
        {
            tmp |= 0x01;
        }
        if( key2_STATUE )
        {
            tmp |= 0x02;
        }
        if( key3_STATUE )
        {
            tmp |= 0x04;
        }
        if( key4_STATUE )
        {
            tmp |= 0x08;
        }
        if( key5_STATUE )
        {
            tmp |= 0x10;
        }
        if( key6_STATUE )
        {
            tmp |= 0x20;
        }
        if( key7_STATUE )
        {
            tmp |= 0x40;
        }
        if( key8_STATUE )
        {
            tmp |= 0x80;
        }
        if( key_value != tmp )
        {
            key_value = tmp;
            i = 0;
        }
    }
    key_value = ~key_value;
    XIAOCHE_GROUP_NUM = key_value;
    XIAOCHE_START_NUM = 1 + 20 * (XIAOCHE_GROUP_NUM - 1);

    MOTO_MAX_COUNT = 20;
#if 0
    if( (max_guandian_count_u16 - XIAOCHE_START_NUM ) > 16 )
    {
        MOTO_MAX_COUNT = 16;
    }
    else
    {
        MOTO_MAX_COUNT = max_guandian_count_u16 - XIAOCHE_START_NUM + 1;
    }
#endif
}
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
    System_Setup();
    memset(Debug_recv_buff_shadow, 0, DEBUG_BUFF_SIZE);
#ifndef SLAVE
    InitSendMsgQueue();
    RX_ZONGXIAN_485;
    zongxian_recv_count = 0;
    //AddSendMsgToQueue(SEND_MSG_CAR2WCS_CMD_TYPE);
#else
    RX_ZONGXIAN_485;
#endif
    RX_DEBUG_485;
    scan_key_process();
    Debug_recv_count = 0;
    //moto_send_count = 1;
    //moto_commu_statue = SEND_DATA;
    //moto_send_process();
    //moto_ready_recv_process();
    write_moto_process();
    /* Infinite loop */
    wcs2carSysEnData.sys_en = 0;
    fun_heart_init();
    init_ethconfig_struct();
    ethinit_configstruct();
    slaverheartmsg.num = 0;
    slaverheartmsg.errnum = 0;
    vphoto_init_slaver_bdcnt_queue();
    vphoto_init_slaver_position_queue();
    vgndfun_init_net();
    vcan_bus_init_cansendqueue();
    while (1)
    {
        if (EthInitStatus != 1)
        {
            Ethernet_Configuration();
            if (EthInitStatus == 1)
            {
                LwIP_Init();
            }
        }
        else
        {
//            check_ETH_link();
//            if(EthInitStatus == 0 )
//            {
//                NVIC_SystemReset();
//            }
            if(ETH_GetRxPktSize() != 0)
            {
                LwIP_Pkt_Handle();
            }
            LwIP_Periodic_Handle(LocalTime);

            udp_client_process();

            send_message_to_sever();
            vgndfun_send_message_to_sever();
        }
//        if( zongxian_commu_state == SENDING_DATA )
//        {
//            if( DMA_GetFlagStatus( DMA1_FLAG_TC7))
//            {
//                zongxian_commu_state = SEND_DATA;
//                DMA_ClearFlag(DMA1_FLAG_TC7);
//
//                DMA_Cmd(DMA1_Channel7, DISABLE);
//                if( zongxian_send_flag == 1 )
//                {
//                    zongxian_send_dely = 20;
//                }
//            }
//        }
//
//        if(zongxian_send_agnain_flag == 1)
//        {
//            zongxian_send_agnain_flag = 0;
//            zongxian_send_process();
//        }
        sec_process();
        
        if(can_send_cnt != can_send_cnt_bk)
        {
//            can_send_frame_process();
            can_send_cnt_bk = can_send_cnt;
        }

        if( DEBUG_recv_flag == 1 )
        {
            DEBUG_recv_flag = 0;
            recv_moto_statue_process();
        }
        vcan_send_frame_process();
        main_onems_process();
    }
}

/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of 10ms periods to wait for.
  * @retval None
  */
void Delay(uint32_t nCount)
{
    // Capture the current local time
    timingdelay = LocalTime + nCount;

    // wait until the desired delay finish
    while(timingdelay > LocalTime)
    {
    }
}

/**
  * @brief  Updates the system local time
  * @param  None
  * @retval None
  */
u16 s_LED_Interval = 0;
u16 s_LED_Local_cnt = 0;

void Time_Update(void)
{

        /*-------------------------------------------*/
        LocalTime += SYSTEMTICK_PERIOD_MS;
        /*-------------------------------------------*/
        
//        Ext_Opt_Process();
        onems_flag = VALUE;

//        if(broadcast_pthoto_over_flag == 1)
//        {
//            RX_DEBUG_485
//        }
//
//        if(broadcast_pthoto_over_flag != 0)
//        {
//            broadcast_pthoto_over_flag--;
//        }
        can_send_cnt++;

        if( sec_reg != 0 )
        {
            sec_reg--;
            if( sec_reg == 0 )
            {
                sec_reg = 1000;
                sec_flag = 1;
            }
        }
        if( TCP_send_dely != 0 )
        {
            TCP_send_dely--;
        }
        if( WCS_reply_timer_out != 0 )
        {
            WCS_reply_timer_out--;
            if( ( udp_send_en == 0 ) && (WCS_reply_timer_out == 0 ) )
            {
                AddSendMsgToQueue(SEND_MSG_CAR2WCS_ONLINE_TYPE);
            }
        }
#ifndef     SLAVE
        WCS_ACK_timer++;

        /*-------------------------------------------*/
//        if( zongxian_send_dely != 0 )
//        {
//            zongxian_send_dely--;
//            if( zongxian_send_dely == 0 )
//            {
//                //zongxian_send_process();
//                zongxian_send_agnain_flag = 1;
//            }
//        }
        /*-------------------------------------------*/
#endif
        /*-------------------------------------------*/

        moto_ctr_process();
        /*-------------------------------------------*/
        if(moto_reply_outtimer != 0 )
        {
            moto_reply_outtimer--;
        }
        /*-------------------------------------------*/
        can_send_timeout();

}

void main_onems_process(void)
{
    if (onems_flag == VALUE) {
        onems_flag = INVALUE;

        photo_deal_with_cnt_photo();
        photo_deal_with_reset_photo();
        photo_trig_config();
        vphoto_upload_slaverbdqueue();
        vphoto_upload_positionqueue();
        vgnd_deal_with_carpos_process();

        //if (slavercntflag == VALUE)
        //{
        //    slavercntflag = INVALUE;
        //    vphoto_recv_slaver_photo_cmd(slaverbdcnt.bdindex, slaverbdcnt.photocnt);
        //}
//        vfun_slaver_heart_increase();

        if (wcs2carSysEnData.sys_en == 1) {
            sysmsg.lostcount++;
            if (sysmsg.lostcount > 10000) {
                sysmsg.cntlost = 1;
                sysmsg.lostcount = 0;
            }

            if (Isphotovalue == 1) {
                phototrig.lostcount++;

                if (phototrig.resetlost >= 5) {
                    phototrig.cntlost = 2;
                }

                if ((phototrig.lostcount > 10000) && (phototrig.resetlost < 5)) {
                    phototrig.cntlost = 1;
                    phototrig.lostcount = 0;
                }
                if ((phototrig.lostcount > 10000) && (phototrig.resetlost >= 5)) {
                    phototrig.cntlost = 3;
                    phototrig.lostcount = 0;
                }
            }

            gndtrig.lostcount++;

            if (gndtrig.resetlost >= 5) {
                gndtrig.cntlost = 2;
            }

            if ((gndtrig.lostcount > 10000) && (gndtrig.resetlost < 5)) {
                gndtrig.cntlost = 1;
                gndtrig.lostcount = 0;
            }
            if ((gndtrig.lostcount > 10000) && (gndtrig.resetlost >= 5)) {
                gndtrig.cntlost = 3;
                gndtrig.lostcount = 0;
            }

        }

        if (slavercntflag == VALUE)
        {
            slavercntflag = INVALUE;
            vphoto_add_to_slaver_bd_queue(slaverbdcnt);
            sysmsg.lostcount = 0;
            if (wcs2carSysEnData.sys_en == 0) {
                return;
            }
            if (slaverbdcnt.error == 0) {
                vphoto_recv_slaver_photo_cmd(slaverbdcnt.bdindex, slaverbdcnt.photocnt);
            }
        }

    }
}



#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {}
}
#endif


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
