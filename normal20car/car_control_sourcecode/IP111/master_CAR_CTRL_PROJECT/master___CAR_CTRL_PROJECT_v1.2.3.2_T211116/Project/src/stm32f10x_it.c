/**
  ******************************************************************************
  * @file    stm32f10x_it.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
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
#include "stm32f10x_it.h"
#include "stm32_eth.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
extern void LwIP_Pkt_Handle(void);

/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    /*while (1)
    {}*/
    NVIC_SystemReset();
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    /*while (1)
    {}*/
    NVIC_SystemReset();
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    /*while (1)
    {}*/
    NVIC_SystemReset();
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void  UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    /*while (1)
    {}*/
    NVIC_SystemReset();
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    /* Update the LocalTime by adding SYSTEMTICK_PERIOD_MS each SysTick interrupt */
    Time_Update();
}
/*************************************************
1MS
*************************************************/
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }

}

extern u8 broadcast_pthoto_over_flag;
void DMA2_Channel5_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA2_FLAG_TC5))
    {
        DMA_ClearFlag(DMA2_FLAG_TC5);  // DMA2_FLAG_GL2      // 清除标志
        DMA_Cmd(DMA2_Channel5, DISABLE);   // 关闭DMA通道

        //RX_DEBUG_485

        broadcast_pthoto_over_flag = 2;  //稍微延迟一点再切换 485 至接收模式，否则会导致最后两个字节发送不完
    }
}

void DMA1_Channel4_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_FLAG_TC4))
    {
        DMA_ClearFlag(DMA1_FLAG_TC4);  // DMA2_FLAG_GL2      // 清除标志
        DMA_Cmd(DMA1_Channel4, DISABLE);   // 关闭DMA通道

        if( moto_commu_statue == SENDING_DATA)
        {
            moto_ready_recv_process();
        }
        // RX_MOTO_485
        // USART_ITConfig(MOTO_USART, USART_IT_TC,ENABLE);

    }
}
/**
  * @brief  This function handles USARTx global interrupt request.
  * @param  None
  * @retval None
  */
extern void recv_photoeletricity();
void DEBUG_IRQHandler(void)
{
    u16 curDataCnt;

    if(USART_GetITStatus(DEBUG_USART, USART_IT_IDLE) != RESET)
    {
        curDataCnt = DEBUG_USART->SR;
        curDataCnt = DEBUG_USART->DR;
        USART_ClearITPendingBit(DEBUG_USART, USART_IT_IDLE);
        curDataCnt = DMA_GetCurrDataCounter(DMA2_Channel3);
        Debug_recv_count = DEBUG_BUFF_SIZE - curDataCnt;
        //debug_uart_recv();
        DEBUG_recv_flag = 1;
        DMA_Cmd(DMA2_Channel3, DISABLE);
#ifdef SLAVE
        recv_photoeletricity();
#endif
        DMA_SetCurrDataCounter(DMA2_Channel3, DEBUG_BUFF_SIZE);
        DMA_Cmd(DMA2_Channel3, ENABLE);
    }

    if( (Debug_recv_buff[0] == 0x55) && (Debug_recv_buff[1] == 0x55) && (Debug_recv_count > 6 ))
    {
        memcpy(Debug_recv_buff_shadow, Debug_recv_buff, Debug_recv_count);
    }

}

/**
  * @brief  This function handles USARTx global interrupt request.
  * @param  None
  * @retval None
  */
void ZONGXIAN_IRQHandler(void)
{

}
/**
  * @brief  This function handles USARTx global interrupt request.
  * @param  None
  * @retval None
  */



void MOTO_IRQHandler(void)
{
    //uint8_t tmp;

    u16 curDataCnt;

    if(USART_GetITStatus(MOTO_USART, USART_IT_IDLE) != RESET)
    {
        if( moto_commu_statue == RECV_DATA)
        {
            curDataCnt = MOTO_USART->SR;
            curDataCnt = MOTO_USART->DR;
            USART_ClearITPendingBit(MOTO_USART, USART_IT_IDLE);
            curDataCnt = DMA_GetCurrDataCounter(DMA1_Channel5);
            moto_recv_count = 20 - curDataCnt;

            DMA_Cmd(DMA1_Channel5, DISABLE);
            moto_recv_now_process();
            DMA_SetCurrDataCounter(DMA1_Channel5, 20);
            DMA_Cmd(DMA1_Channel5, ENABLE);
        }
    }
    else if(USART_GetITStatus(MOTO_USART, USART_IT_TC) != RESET)
    {
        if( moto_commu_statue == SENDING_DATA)
        {
            // LED_OFF;
            //   moto_ready_recv_process();
            USART_ITConfig(MOTO_USART, USART_IT_TC, DISABLE);
            USART_ClearFlag(MOTO_USART, USART_FLAG_TC);
        }

    }
}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles ETH interrupt request.
  * @param  None
  * @retval None
  */
//void ETH_IRQHandler(void)
//{
//    /* Handles all the received frames */
////    while(ETH_GetRxPktSize() != 0)
////    {
////        LwIP_Pkt_Handle();
////    }
//
//    /* Clear the Eth DMA Rx IT pending bits */
//    ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
//    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
//}

/**
  * @brief  This function handles External lines 15 to 10 interrupt request.
  * @param  None
  * @retval None
  */

//void EXTI9_5_IRQHandler(void)
//{
//    u16 i;
//
//    if(EXTI_GetITStatus(EXTI_Line6) != RESET)
//    {
//        EXTI_ClearITPendingBit(EXTI_Line6);
//        for(i = 0; i < 20; i++)
//        {
//            if(INPUT2_STATUE == 0)
//                break;
//        }
//        if(i == 20)
//        {
//            //Photoeletricity_Process(INT_OPT_INC);
//            opt_inc_flag = 1;
//            opt_inc_cnt = 0;
//        }
//    }
//    if(EXTI_GetITStatus(EXTI_Line7) != RESET)
//    {
//        EXTI_ClearITPendingBit(EXTI_Line7);
//        for(i = 0; i < 20; i++)
//        {
//            if(INPUT3_STATUE == 0)
//                break;
//        }
//        if(i == 20)
//        {
//            //Photoeletricity_Process(INT_OPT_RST);
//            opt_rst_flag = 1;
//            opt_rst_cnt = 0;
//        }
//    }
//
//}

void CAN1_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);
    can_bus_frame_receive(RxMessage);
}
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
