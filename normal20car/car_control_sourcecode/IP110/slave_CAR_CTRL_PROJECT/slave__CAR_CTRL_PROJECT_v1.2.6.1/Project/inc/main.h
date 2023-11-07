/**
******************************************************************************
* @file    main.h
* @author  MCD Application Team
* @version V1.0.0
* @date    11/20/2009
* @brief   This file contains all the functions prototypes for the main.c
*          file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

    /* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f107.h"
#include "fun.h"
#include "can_bus.h"
#include <string.h>
#include "photo.h"

    //#define _UDP_DEBUG_  1

#define     USE_UDP
#define MAX_GUANGDIAN_COUNT 1024
#define SLAVE
    //#define SLAVE_TEST_ZONGXIAN
    //#define ZIDONG_XIAOLIAO
    //#define ZIDONG_LOAD
    //#define GUANGDIAN_TEST
    //#define     LOAD_TEST
    //#define     WIFI_TEST
    //#define     MOTO_TEST_MODE      1
    //#define     ZONGXIAN_TEST_MODE      1
    //#define     GD_TEST_MODE        1
#define MAX_LOAD_RUN_LENGTH_RATIO  1.1     //上料时，滚筒最大转动距离为 1.1 个车

    /* Exported function prototypes ----------------------------------------------*/
    void Time_Update(void);
    void Delay(uint32_t nCount);

    /* ETHERNET errors */
#define  ETH_ERROR              ((uint32_t)0)
#define  ETH_SUCCESS            ((uint32_t)1)
#define  DP83848_PHY_ADDRESS       0x01

    //#define	TCP_RECEV_BUFF_SIZE		500
    //#define	TCP_SEND_BUFF_SIZE		500
#define	TCP_RECEV_BUFF_SIZE		2048
#define	TCP_SEND_BUFF_SIZE		500
#define	CLIENT_DIS_CONNECT		0
#define	CLIENT_CONNECT_OK		1
#define	CLIENT_CONNECT_RECV		2
#define	CLIENT_RE_CONNECT		3

#define DEST_IP_ADDR0   192
#define DEST_IP_ADDR1   168
#define DEST_IP_ADDR2   10
#define DEST_IP_ADDR3   12
#define DEST_PORT       9000

    /* MAC ADDRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
#define MAC_ADDR0   0
#define MAC_ADDR1   0
#define MAC_ADDR2   0
#define MAC_ADDR3   0
#define MAC_ADDR4   0
#define MAC_ADDR5   1
    /*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   10
#define IP_ADDR3   110
    /*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0
    /*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   10
#define GW_ADDR3   1
    extern void  DEBUG_process(u8 *p_data, u16 len);

#define	 key1_STATUE	        GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)
#define	 key2_STATUE	        GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)
#define	 key3_STATUE	        GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)
#define	 key4_STATUE	        GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)

#define	 key5_STATUE	        GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)
#define	 key6_STATUE	        GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0)
#define	 key7_STATUE	        GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)
#define	 key8_STATUE	        GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7)

#define	 LED_STATUE		GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_2)
#define	 LED_ON			GPIO_ResetBits(GPIOE,GPIO_Pin_2)
#define	 LED_OFF		GPIO_SetBits(GPIOE,GPIO_Pin_2)

#define	 INPUT2_STATUE	        GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6)
#define	 INPUT3_STATUE	        GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7)

#define  USART2_DR_Base         0x40004404
#define  USART1_DR_Base         0x40013804
#define  DEBUG_USART		UART4
#define  ZONGXIAN_USART		USART2
#define  MOTO_USART		USART1 

#define  DEBUG_IRQn		UART4_IRQn
#define  ZONGXIAN_IRQn	        USART2_IRQn
#define  MOTO_IRQn		USART1_IRQn

#define	 DEBUG_IRQHandler	UART4_IRQHandler
#define	 ZONGXIAN_IRQHandler	USART2_IRQHandler
#define	 MOTO_IRQHandler	USART1_IRQHandler

#define	 RX_DEBUG_485		GPIO_ResetBits( GPIOD, GPIO_Pin_7);
#define	 TX_DEBUG_485		GPIO_SetBits( GPIOD, GPIO_Pin_7);

#define	 RX_ZONGXIAN_485	GPIO_ResetBits( GPIOD, GPIO_Pin_4);
#define	 TX_ZONGXIAN_485	GPIO_SetBits( GPIOD, GPIO_Pin_4);

#define	 RX_MOTO_485		GPIO_ResetBits( GPIOA, GPIO_Pin_11);
#define	 TX_MOTO_485		GPIO_SetBits( GPIOA, GPIO_Pin_11);

#define	 RECV_DATA		1
#define	 SEND_DATA		2
#define  SENDING_DATA           3
#define	 RECV_DATA_END	        4
    
extern u8 DEBUG_recv_flag;
extern u8 heart_dely;
extern void WIFI_test_process(void);

#define     CAR_LENGTH          650  //mm
//#define     XIAOCHE_KUANDU      600

enum
{
    MOTO_OPT_NO_CMD    = 0,
    MOTO_OPT_SEND_PARA = 1,
    MOTO_OPT_RUN_EN    = 2
};

#define	POSTION_JIANGE	    1
#define MOTO_MAX_COUNT1     20
#define SEND_PARA_STATUE    1
#define WAIT_RECV_PARA      2
#define WAIT_RECV_RUN       3

extern u8  Isphotovalue;
extern u32 s_LastPhotoCount;

    extern u8 key_value;
    
    extern u8 check_motor_flag;
    extern uint8_t     XIAOCHE_GROUP_NUM;
    extern uint16_t	    XIAOCHE_START_NUM;
    extern uint8_t     MOTO_MAX_COUNT;
    extern moto_para_struct   moto_para[];
    extern uint8_t moto_send_buff[];
    extern uint8_t moto_recv_buff[];
    extern uint8_t  cur_moto_send_count;
    extern uint8_t moto_send_count;
    extern uint8_t moto_recv_count;
    extern uint8_t moto_commu_statue;
    extern uint8_t moto_recv_dely;
    extern uint8_t	moto_reply_outtimer;
    extern uint8_t moto_start_ctr;
    extern sLoad_info Load_info_t;
    extern sUnload_info Unload_info_t;
    extern u16 max_guandian_count_u16;
    extern void moto_ready_recv_process(void);
    extern void moto_send_process(void);
    extern void moto_recv_process(void);
    extern void moto_recv_now_process(void);
    extern void write_moto_process(void);
    extern void  send_moto_para_process(uint8_t moto_num);
    extern void moto_ctr_process(void);
    extern void xialiao_process(void);
    extern void  get_xialiao_num_process(void);
    extern void  get_load_num(void);
    extern void moto_message_process(void);
    extern void motor_position_reset(void);
    //guangdian.c
    extern uint8_t     guangdian_error;
    extern u16 INPUT2_count_error;
    extern uint32_t	INPUT2_L_timer;
    extern uint32_t INPUT2_H_timer;
    extern uint32_t	INPUT2_timer;
    extern uint16_t	INPUT2_timer_beilv;
    extern uint16_t	INPUT2_count;
    extern uint16_t pre_INPUT2_count;
    extern u8 moto_statue_comm_en;

    extern void  jisuan_guangdian_timer( void );
    extern u8 LOAD_test_dely;
#define ZONGXIAN_BUFF_SIZE  1000//500
#define DEBUG_BUFF_SIZE     50
    extern u8  zongxian_send_buff[];
    extern u8  zongxian_recv_buff[];
    extern u16 zongxian_send_count;
    extern u16 zongxian_recv_count;
    extern u8 Debug_recv_buff[];
    extern u8 Debug_recv_count;
    extern u8 DEBUG_USART_tmr;
    extern u16 statue_car_num;
    extern u8  statue_car_count;
    extern u16 statue_guangdian_num;
    extern u8  zongxian_commu_state;
    extern u8  zongxian_tmr;
    extern u8 zongxian_send_flag;
    extern u8 zongxian_send_dely;
    extern void zongxian_send_process(void);
    extern void zongxian_uart_broadcast(u8 *point, u16 msg_type);
    extern void zongxian_uart_recv();
    extern void slave_send_moto_process(void);
    extern void recv_moto_statue_process(void);
    extern void recv_photoeletricity();
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

