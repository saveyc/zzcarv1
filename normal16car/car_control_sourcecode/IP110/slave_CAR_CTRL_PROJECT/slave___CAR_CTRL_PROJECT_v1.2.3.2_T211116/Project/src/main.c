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
__IO uint32_t LocalTime_half_ms = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;
u16 sec_reg = 1000;
u16 led_on_delay = 0;
u8 slave_send_debug_flag = 0;
u16 debug_message_dely = 200;
u16  sec_flag = 0;
u8 DEBUG_recv_flag = 0;
u8 key_value = 0;
u16 max_guandian_count_u16 = 0;
u8 check_motor_flag = 0;
u32  memery_back[4000] = {0};
/* Private function prototypes -----------------------------------------------*/
void System_Periodic_Handle(void);
extern u8 zongxian_send_agnain_flag;

extern void Broadcast_Photoeletricity(u8 level, u8 error_flag, uint16_t INPUT2_count, u32 zhuxian_speed);

void main_cualulat(void)
{
  u16 i = 1;
  u16 j = 1;
  u16 k = 0;
  
  k = j / i;
}
//1s
void sec_process(void)
{
    u8 buf[10] = { 0 };
    if( sec_flag == 1 )
    {
        sec_flag = 0;
        /*if( INPUT2_STATUE )
        {
            LED_ON;
        }
        else
        {
            LED_OFF;
        } */

        if ((sysmsg.cntlost != 0) && (Isphotovalue == 1)) {

            buf[0] = sysmsg.curposition & 0xFF;
            buf[1] = (sysmsg.curposition >> 8) & 0xFF;
            buf[2] = sysmsg.interval & 0xFF;
            buf[3] = (sysmsg.interval >> 8) & 0xFF;
            buf[4] = sysmsg.cntlost & 0xFF;
            buf[5] = (sysmsg.cntlost >> 8) & 0xFF;

            vcan_send_send_msg(buf, 6, CAN_FUNC_ID_SLAVER_PHOTO_CNT_TYPE, 1);

            sysmsg.cntlost = 0;
        }

        vcanbus_send_heart_msg();


        if( heart_dely != 0 )
        {
            heart_dely--;
        }
        
#ifdef  MOTO_TEST_MODE
        moto_start_ctr = 1;
#endif
        
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
    XIAOCHE_START_NUM = 1 + 16 * (XIAOCHE_GROUP_NUM - 1);

    MOTO_MAX_COUNT = 16;
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

    //RX_ZONGXIAN_485;
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
    vphoto_init_msg();
    vcan_bus_init_cansendqueue();
    while (1)
    {
        //zongxian_uart_recv();
        slave_send_moto_process();
        sec_process();
        if( DEBUG_recv_flag == 1 )
        {
            DEBUG_recv_flag = 0;
            recv_moto_statue_process();
        }
        vcan_send_frame_process();
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
extern u32 s_load_unload_packet;
extern u16 s_guangdian_recover_timer_16;
void Time_Update(void)
{
    // LocalTime_half_ms++;

    // if((LocalTime_half_ms & 1) == 0)
    {
        /*-------------------------------------------*/
        LocalTime += SYSTEMTICK_PERIOD_MS;
        /*-------------------------------------------*/
        if(led_on_delay == 1)
        {
            LED_OFF;
        }
        if(led_on_delay != 0)
        {
            led_on_delay--;
        }

        if( wcs2carSysEnData.sys_en == 1 )
        {
            if(slave_send_debug_flag == 1)
            {
                moto_statue_comm_en = 1;
            }
        }

        if(slave_send_debug_flag != 0)
        {
            slave_send_debug_flag--;
        }

        if( sec_reg != 0 )
        {
            sec_reg--;
            if( sec_reg == 0 )
            {
                sec_reg = 1000;
                sec_flag = 1;
            }
        }
        
        /*-------------------------------------------*/
//        if(s_guangdian_recover_timer_16 > 0)
//        {
//            s_guangdian_recover_timer_16--;
//        }
//
//        if(s_load_unload_packet > 0)
//        {
//            s_load_unload_packet--;
//        }

//        if((wcs2carSysEnData.sys_en == 1) && (s_load_unload_packet == 1))
//        {
//            INPUT2_count_error |= 0x400;
//        }


//        if((wcs2carSysEnData.sys_en == 1) && (s_guangdian_recover_timer_16 == 1))
//        {
//            INPUT2_count_error |= 0x1000;
//            INPUT2_count = (INPUT2_count + 1) > max_guandian_count_u16 ? 1 : (INPUT2_count + 1);
//            xialiao_process();
//        }
        moto_ctr_process();
        /*-------------------------------------------*/
        if(moto_reply_outtimer != 0 )
        {
            moto_reply_outtimer--;
        }
        vphoto_deal_with_reset_photo();
        vphoto_deal_with_cnt_photo();

        if ((wcs2carSysEnData.sys_en == 1) && (Isphotovalue == 1)) {


            sysmsg.lostcount++;

            if (sysmsg.resetlost >= 5) {
                sysmsg.cntlost = 2;
            }

            if ((sysmsg.lostcount > 10000) && (sysmsg.resetlost < 5)) {
                sysmsg.cntlost = 1;
                sysmsg.lostcount = 0;
            }
            if ((sysmsg.lostcount > 10000) && (sysmsg.resetlost >= 5)) {
                sysmsg.cntlost = 3;
                sysmsg.lostcount = 0;
            }

        }
        /*-------------------------------------------*/
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
