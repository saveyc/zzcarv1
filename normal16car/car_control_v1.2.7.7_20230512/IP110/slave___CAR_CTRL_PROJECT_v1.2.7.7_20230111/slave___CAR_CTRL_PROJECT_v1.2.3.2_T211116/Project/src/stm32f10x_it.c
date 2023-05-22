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
#ifndef      GD_TEST_MODE
        if( LED_STATUE )
        {
            // LED_ON;
        }
        else
        {
            //   LED_OFF;//GWF
        }
#endif
        INPUT2_timer_beilv++;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
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
    }
}
/**
  * @brief  This function handles USARTx global interrupt request.
  * @param  None
  * @retval None
  */
void boot_mode_cmd_check()
{
    u8 i, sum = 0;

    if(Debug_recv_count != 5) return;
    if(Debug_recv_buff[0] != 0xAA || Debug_recv_buff[1] != 0xAA) return;
    if(Debug_recv_buff[2] != key_value) return;
    if(Debug_recv_buff[3] != 0xB0) return;
    for(i = 0; i < 4 ; i++)
    {
        sum += Debug_recv_buff[i];
    }
    if(Debug_recv_buff[4] != sum) return;

    BKP_WriteBackupRegister(BKP_DR8, 0x55);
    NVIC_SystemReset();
}
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
        boot_mode_cmd_check();
        DEBUG_recv_flag = 1;
        DMA_Cmd(DMA2_Channel3, DISABLE);
        //recv_photoeletricity();
        DMA_SetCurrDataCounter(DMA2_Channel3, DEBUG_BUFF_SIZE);
        DMA_Cmd(DMA2_Channel3, ENABLE);
    }

}

/**
  * @brief  This function handles USARTx global interrupt request.
  * @param  None
  * @retval None
  */
void ZONGXIAN_IRQHandler(void)
{

#ifdef SLAVE
    u16 curDataCnt;

    if(USART_GetITStatus(ZONGXIAN_USART, USART_IT_IDLE) != RESET)
    {
        curDataCnt = ZONGXIAN_USART->SR;
        curDataCnt = ZONGXIAN_USART->DR;
        USART_ClearITPendingBit(ZONGXIAN_USART, USART_IT_IDLE);
        curDataCnt = DMA_GetCurrDataCounter(DMA1_Channel6);
        zongxian_recv_count = ZONGXIAN_BUFF_SIZE - curDataCnt;
        //zongxian_commu_state = RECV_DATA_END;
        zongxian_uart_recv();
        DMA_Cmd(DMA1_Channel6, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel6, ZONGXIAN_BUFF_SIZE);
        DMA_Cmd(DMA1_Channel6, ENABLE);
    }
#endif
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
            //  LED_OFF;
            //    moto_ready_recv_process();
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
void ETH_IRQHandler(void)
{
    /* Handles all the received frames */
    while(ETH_GetRxPktSize() != 0)
    {
        LwIP_Pkt_Handle();
    }

    /* Clear the Eth DMA Rx IT pending bits */
    ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}


/**
  * @brief  This function handles External lines 15 to 10 interrupt request.
  * @param  None
  * @retval None
  */
extern __IO uint32_t LocalTime_half_ms;
void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        INPUT2_timer = LocalTime_half_ms; // unit : 0.5 ms    //    TIM2->CNT;
        //jisuan_guangdian_timer();
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
}

void CAN1_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);
    can_bus_frame_receive(RxMessage);
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
