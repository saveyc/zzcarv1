/**
  ******************************************************************************
  * @file    stm32f107.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   STM32F107 hardware configuration
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
#include "stm32f107.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DP83848_PHY        /* Ethernet pins mapped on STM3210C-EVAL Board */
#define PHY_ADDRESS       0x01 /* Relative to STM3210C-EVAL Board */

#define MII_MODE          /* MII mode for STM3210C-EVAL Board (MB784) (check jumpers setting) */
//#define RMII_MODE       /* RMII mode for STM3210C-EVAL Board (MB784) (check jumpers setting) */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void Ethernet_Configuration(void);
extern u8 Debug_send_buff[DEBUG_BUFF_SIZE];

__IO uint32_t  EthInitStatus = 0;
ETH_InitTypeDef ETH_InitStructure;
eth_config ethconfig;

void init_ethconfig_struct(void)
{
    ethconfig.firststep = INVALUE;
    ethconfig.secondstep = INVALUE;
    ethconfig.thirdstep = INVALUE;
}
/***************************************************************************

***************************************************************************/
void TIM_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    /************************************************定时器4设置********************************************************/
    /* 基础设置*/
    TIM_TimeBaseStructure.TIM_Period = 65535;  			//1MS
    TIM_TimeBaseStructure.TIM_Prescaler = 3600;    	//预分频    50us计时单位
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 	//向上计数
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /*使能预装载*/
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    /*预先清除所有中断位*/
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    /*溢出都配置中断*/
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    /* 允许TIM2开始计数 */
    //TIM_Cmd(TIM2, ENABLE);  modified by gwf   source: TIM_Cmd(TIM2, ENABLE);
}
/***************************************************************************

****************************************************************************/
void EXTI_Configuration(void)
{
#ifndef     SLAVE
    EXTI_InitTypeDef EXTI_InitStructure;

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
    EXTI_ClearITPendingBit(EXTI_Line6);
    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;	//上升沿触发中断
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);
    EXTI_ClearITPendingBit(EXTI_Line7);
    EXTI_InitStructure.EXTI_Line = EXTI_Line7;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;	//上升沿触发中断
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
#endif
}
/*****************************************************************************8

*****************************************************************************/
void USART_Configuration(void)
{
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = 38400;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(DEBUG_USART, &USART_InitStructure);
    //USART_ITConfig(DEBUG_USART, USART_IT_RXNE, ENABLE);
#ifndef SLAVE
    USART_DMACmd(DEBUG_USART, USART_DMAReq_Tx, ENABLE);//USART_DMACmd(DEBUG_USART, USART_DMAReq_Tx, DISABLE);   //
    //added by gwf 2016.11.9  for master broadcast pthotoeletricity
#else
    USART_DMACmd(DEBUG_USART, USART_DMAReq_Tx, DISABLE);
#endif
    USART_DMACmd(DEBUG_USART, USART_DMAReq_Rx, ENABLE);
    USART_ITConfig(DEBUG_USART, USART_IT_IDLE, ENABLE);
    USART_Cmd(DEBUG_USART, ENABLE);
//    /*******************************************************/
//    USART_InitStructure.USART_BaudRate = 38400;
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//    USART_InitStructure.USART_StopBits = USART_StopBits_1;
//    USART_InitStructure.USART_Parity = USART_Parity_No;
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//
//    USART_Init(ZONGXIAN_USART, &USART_InitStructure);
//    //USART_ITConfig(ZONGXIAN_USART, USART_IT_RXNE,ENABLE);
//    //USART_ITConfig(ZONGXIAN_USART, USART_IT_TC,ENABLE);
//    USART_DMACmd(ZONGXIAN_USART, USART_DMAReq_Tx, DISABLE);
//#ifdef SLAVE
//    USART_DMACmd(ZONGXIAN_USART, USART_DMAReq_Rx, ENABLE);
//    USART_ITConfig(ZONGXIAN_USART, USART_IT_IDLE, ENABLE);
//#endif
//    USART_Cmd(ZONGXIAN_USART, ENABLE);

    USART_InitStructure.USART_BaudRate = 38400;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(MOTO_USART, &USART_InitStructure);
    USART_DMACmd(MOTO_USART, USART_DMAReq_Tx, ENABLE);//USART_DMACmd(DEBUG_USART, USART_DMAReq_Tx, DISABLE);   //
    USART_DMACmd(MOTO_USART, USART_DMAReq_Rx, ENABLE);
    USART_ITConfig(MOTO_USART, USART_IT_IDLE, ENABLE);

    USART_Cmd(MOTO_USART, ENABLE);
}

/****************************************************************************

******************************************************************************/
void DMA_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;
//    /* DMA1 Channel7 (triggered by USART2 Tx event) Config */
//    DMA_DeInit(DMA1_Channel7);
//    DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Base;
//    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)zongxian_send_buff;
//    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
//    DMA_InitStructure.DMA_BufferSize = 32;
//    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
//    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//
//    DMA_Init(DMA1_Channel7, &DMA_InitStructure);
//    DMA_ClearFlag(DMA1_FLAG_TC7);
//    DMA_Cmd(DMA1_Channel7, DISABLE);
//#ifdef SLAVE
//    DMA_DeInit(DMA1_Channel6);
//    DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Base;
//    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)zongxian_recv_buff;
//    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//    DMA_InitStructure.DMA_BufferSize = ZONGXIAN_BUFF_SIZE;
//    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
//    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//
//    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
//    DMA_Cmd(DMA1_Channel6, ENABLE);
//#endif
    /* DMA2 Channel3 */
    DMA_DeInit(DMA2_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART4->DR);//USART4_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Debug_recv_buff;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = DEBUG_BUFF_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA2_Channel3, &DMA_InitStructure);
    DMA_Cmd(DMA2_Channel3, ENABLE);
#ifndef SLAVE
    DMA_DeInit(DMA2_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART4->DR);;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Debug_send_buff;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = DEBUG_BUFF_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA2_Channel5, &DMA_InitStructure);
    DMA_ClearFlag(DMA2_FLAG_TC5);
    DMA_Cmd(DMA2_Channel5, DISABLE);
    DMA_ITConfig(DMA2_Channel5, DMA_IT_TC, ENABLE);

#endif
    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)moto_send_buff;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 20;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel4, &DMA_InitStructure);
    DMA_ClearFlag(DMA1_FLAG_TC4);
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);

    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);//USART4_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)moto_recv_buff;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 20;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel5, ENABLE);
}

void CAN_Configuration(void)
{
    CAN_InitTypeDef        CAN_InitStructure;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;

    /* CAN register init */
    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

    /* CAN cell init */
    /* CAN cell init */
    CAN_InitStructure.CAN_TTCM = DISABLE;		//时间触发
    CAN_InitStructure.CAN_ABOM = ENABLE;		//自动离线管理
    CAN_InitStructure.CAN_AWUM = DISABLE;		//自动唤醒
    CAN_InitStructure.CAN_NART = DISABLE;		//自动重传
    CAN_InitStructure.CAN_RFLM = DISABLE;               //接收FIFO锁定
    CAN_InitStructure.CAN_TXFP = ENABLE;                //发送FIFO模式
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	//正常传输模式
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;    //SamplePoint  (1+13) / (1+13+2) = 87.5%
    CAN_InitStructure.CAN_Prescaler = 18;       //BaudRate   36M /18/(1+13+2) = 125kbps
    CAN_Init(CAN1, &CAN_InitStructure);

    /* CAN 过滤器设置 */
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);

    /* 允许FMP0中断   FIFO消息挂号*/
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}

/**
  * @brief  Setup STM32 system (clocks, Ethernet, GPIO, NVIC) and STM3210C-EVAL resources.
  * @param  None
  * @retval None
  */
void System_Setup(void)
{
    RCC_ClocksTypeDef RCC_Clocks;

    /* Setup STM32 clock, PLL and Flash configuration) */
    SystemInit();

    /* Enable USART2 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_TIM2 | RCC_APB1Periph_USART2 | RCC_APB1Periph_UART4 | RCC_APB1Periph_UART5
                           | RCC_APB1Periph_PWR | RCC_APB1Periph_CAN1, ENABLE);


    /* Enable ETHERNET clock  */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_DMA2 | RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |
                          RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

    /* Enable GPIOs and ADC1 clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_USART1 |
                           RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO , ENABLE);

    /* NVIC configuration */
    NVIC_Configuration();

    /* Configure the GPIO ports */
    GPIO_Configuration();
//    EXTI_Configuration();
    USART_Configuration();
    DMA_Configuration();
    CAN_Configuration();
    //TIM_Configuration();
    /* Configure the Ethernet peripheral */
    //Ethernet_Configuration();

    /* SystTick configuration: an interrupt every 10ms */
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.SYSCLK_Frequency / 1000);

    /* Update the SysTick IRQ priority should be higher than the Ethernet IRQ */
    /* The Localtime should be updated during the Ethernet packets processing */
    NVIC_SetPriority (SysTick_IRQn, 2);
}

/**
  * @brief  Configures the Ethernet Interface
  * @param  None
  * @retval None
  */
void Ethernet_Configuration(void)
{
    if (ethconfig.firststep == INVALUE) {
        /* MII/RMII Media interface selection ------------------------------------------*/
#ifdef MII_MODE /* Mode MII with STM3210C-EVAL  */
        GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_MII);

        /* Get HSE clock = 25MHz on PA8 pin (MCO) */
        RCC_MCOConfig(RCC_MCO_HSE);

#elif defined RMII_MODE  /* Mode RMII with STM3210C-EVAL */
        GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);

        /* Set PLL3 clock output to 50MHz (25MHz /5 *10 =50MHz) */
        RCC_PLL3Config(RCC_PLL3Mul_10);
        /* Enable PLL3 */
        RCC_PLL3Cmd(ENABLE);
        /* Wait till PLL3 is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET)
        {
        }

        /* Get PLL3 clock on PA8 pin (MCO) */
        RCC_MCOConfig(RCC_MCO_PLL3CLK);
#endif

        /* Reset ETHERNET on AHB Bus */
        ETH_DeInit();

        /* Software reset */
        ETH_SoftwareReset();

        ethconfig.firststep = VALUE;
    }

    if (ethconfig.secondstep == INVALUE) {
        if (ETH_GetSoftwareResetStatus() == SET)
        {
            return;
        }
        else {
            ethconfig.secondstep = VALUE;
        }
    }

    if (ethconfig.thirdstep == INVALUE) {
        /* ETHERNET Configuration ------------------------------------------------------*/
/* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
        ETH_StructInit(&ETH_InitStructure);

        /* Fill ETH_InitStructure parametrs */
        /*------------------------   MAC   -----------------------------------*/
        ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
        ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
        ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
        ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
        ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
        ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
        ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
        ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
        ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
        ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif

        /*------------------------   DMA   -----------------------------------*/

        /* When we use the Checksum offload feature, we need to enable the Store and Forward mode:
        the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum,
        if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
        ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
        ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
        ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;

        ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
        ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
        ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
        ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
        ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
        ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
        ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
        ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
    }

    /* Configure Ethernet */
    EthInitStatus = ETH_Init(&ETH_InitStructure, PHY_ADDRESS);

    if (EthInitStatus == 2) {
        return;
    }

    if (EthInitStatus == 0) {
        init_ethconfig_struct();
        ethinit_configstruct();
    }

    /* Enable the Ethernet Rx Interrupt */
    ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);

}
/**************************************************************8

****************************************************************/
uint32_t check_link(uint16_t PHYAddress)
{
    if( !(ETH_ReadPHYRegister(PHYAddress, PHY_BSR) & PHY_Linked_Status) )
    {
        return ETH_ERROR;
    }
    else
    {
        return ETH_SUCCESS;
    }
}
void  check_ETH_link(void)
{
    EthInitStatus = check_link(DP83848_PHY_ADDRESS);
}
/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* ETHERNET pins configuration */
    /* AF Output Push Pull:
    - ETH_MII_MDIO / ETH_RMII_MDIO: PA2
    - ETH_MII_MDC / ETH_RMII_MDC: PC1
    - ETH_MII_TXD2: PC2
    - ETH_MII_TX_EN / ETH_RMII_TX_EN: PB11
    - ETH_MII_TXD0 / ETH_RMII_TXD0: PB12
    - ETH_MII_TXD1 / ETH_RMII_TXD1: PB13
    - ETH_MII_PPS_OUT / ETH_RMII_PPS_OUT: PB5
    - ETH_MII_TXD3: PB8 */

    /* Configure PA2 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure PC1, PC2 and PC3 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure PB5, PB8, PB11, PB12 and PB13 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_11 |
                                  GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /**************************************************************/
    /*               For Remapped Ethernet pins                   */
    /*************************************************************/
    /* Input (Reset Value):
    - ETH_MII_CRS CRS: PA0
    - ETH_MII_RX_CLK / ETH_RMII_REF_CLK: PA1
    - ETH_MII_COL: PA3
    - ETH_MII_RX_DV / ETH_RMII_CRS_DV: PD8
    - ETH_MII_TX_CLK: PC3
    - ETH_MII_RXD0 / ETH_RMII_RXD0: PD9
    - ETH_MII_RXD1 / ETH_RMII_RXD1: PD10
    - ETH_MII_RXD2: PD11
    - ETH_MII_RXD3: PD12
    - ETH_MII_RX_ER: PB10 */

    /* ETHERNET pins remapp in STM3210C-EVAL board: RX_DV and RxD[3:0] */
    GPIO_PinRemapConfig(GPIO_Remap_ETH, ENABLE);

    /* Configure PA0, PA1 and PA3 as input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure PB10 as input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure PC3 as input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure PD8, PD9, PD10, PD11 and PD12 as input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* MCO pin configuration------------------------------------------------- */
    /* Configure MCO (PA8) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //LED
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    LED_ON;

    //初始化串口4引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //初始化串口2引脚
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    //初始化串口1引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //485
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //input
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
#if 0
    //output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC, GPIO_Pin_6);
    GPIO_ResetBits(GPIOC, GPIO_Pin_6);
#endif
    //KEY
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    //CAN RX 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    //CAN TX 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_PinRemapConfig(GPIO_Remap2_CAN1,ENABLE);
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Set the Vector Table base location at 0x08000000 */
#ifdef	IAP_MODE_SYS
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x10000);
#else
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
#endif

    /* 2 bit for pre-emption priority, 2 bits for subpriority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* Enable the Ethernet global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = DEBUG_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

//    NVIC_InitStructure.NVIC_IRQChannel = ZONGXIAN_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = MOTO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#ifndef SLAVE
    //ADDED BY GWF
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel5_IRQn;   // 发送DMA通道的中断配置
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;     // 优先级设置
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;   // 发送DMA通道的中断配置
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;     // 优先级设置
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    ////NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    //NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    //NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    //NVIC_Init(&NVIC_InitStructure);

    /* Timer2中断*/
//    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
    
    /* CAN-RX*/
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
