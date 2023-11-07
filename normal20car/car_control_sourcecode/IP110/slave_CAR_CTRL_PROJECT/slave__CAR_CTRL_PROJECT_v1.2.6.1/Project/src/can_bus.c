#include "stm32f10x.h"
#include "stm32f107.h"
#include "main.h"

u8  can_recv_buff[CAN_RX_BUFF_SIZE];
u16 can_recv_len = 0;
u8  g_SegPolo = CAN_SEG_POLO_NONE;
u8  g_SegNum = 0;
u16 g_SegBytes = 0;

#define  canframebuffsize   50
sCanFrameExt    canframequeuebuff[canframebuffsize];
sCanFrameQueue   canframequeue;

void vcan_bus_init_cansendqueue(void)
{
    sCanFrameQueue* q = NULL;
    q = &canframequeue;

    q->queue = canframequeuebuff;
    q->front = q->rear = 0;
    q->maxsize = canframebuffsize;
}

void vcan_bus_addto_cansendqueue(sCanFrameExt x)
{
    sCanFrameQueue* q = NULL;
    q = &canframequeue;

    if ((q->rear + 1) % q->maxsize == q->front)
    {
        return;
    }

    q->rear = (q->rear + 1) % q->maxsize;

    q->queue[q->rear] = x;
}


u8 u8can_bus_send_one_frame(sCanFrameExt sTxMsg)
{
    CanTxMsg TxMessage;

    TxMessage.ExtId = (sTxMsg.extId.mac_id & 0xFF) | ((sTxMsg.extId.func_id & 0xF) << 8) | ((sTxMsg.extId.seg_num & 0xFF) << 12) | ((sTxMsg.extId.seg_polo & 0x3) << 20) | ((sTxMsg.extId.dst_id & 0x7F) << 22);
    TxMessage.IDE = CAN_ID_EXT;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = sTxMsg.data_len;
    memcpy(TxMessage.Data, sTxMsg.data, TxMessage.DLC);

    return CAN_Transmit(CAN1, &TxMessage);
}

void vcan_send_send_msg(u8* buff, u8 send_total_len, u8 type, u8 dst)
{
    sCanFrameExt canTxMsg;
    u8 send_len = 0;

    canTxMsg.extId.func_id = type;
    canTxMsg.extId.mac_id = XIAOCHE_GROUP_NUM;
    canTxMsg.extId.dst_id = dst;

    if (send_total_len <= CAN_PACK_DATA_LEN)//不需分段传输
    {
        canTxMsg.extId.seg_polo = CAN_SEG_POLO_NONE;
        canTxMsg.extId.seg_num = 0;
        canTxMsg.data_len = send_total_len;
        memcpy(canTxMsg.data, buff, canTxMsg.data_len);
        vcan_bus_addto_cansendqueue(canTxMsg);
    }
    else//需要分段传输
    {
        canTxMsg.extId.seg_polo = CAN_SEG_POLO_FIRST;
        canTxMsg.extId.seg_num = 0;
        canTxMsg.data_len = CAN_PACK_DATA_LEN;
        memcpy(canTxMsg.data, buff, canTxMsg.data_len);
        vcan_bus_addto_cansendqueue(canTxMsg);
        send_len += CAN_PACK_DATA_LEN;
        while (1) {
            if (send_len + CAN_PACK_DATA_LEN < send_total_len)
            {
                canTxMsg.extId.seg_polo = CAN_SEG_POLO_MIDDLE;
                canTxMsg.extId.seg_num++;
                canTxMsg.data_len = CAN_PACK_DATA_LEN;
                memcpy(canTxMsg.data, buff + send_len, canTxMsg.data_len);
                vcan_bus_addto_cansendqueue(canTxMsg);
                send_len += CAN_PACK_DATA_LEN;
            }
            else
            {
                canTxMsg.extId.seg_polo = CAN_SEG_POLO_FINAL;
                canTxMsg.extId.seg_num++;
                canTxMsg.data_len = send_total_len - send_len;
                memcpy(canTxMsg.data, buff + send_len, canTxMsg.data_len);
                vcan_bus_addto_cansendqueue(canTxMsg);
                break;
            }
        }
    }
}

void vcan_send_frame_process(void)
{
    sCanFrameQueue* q = &canframequeue;
    sCanFrameExt* canTxMsg = NULL;
    u16 front = 0;
    u8  tx_mailbox = 0;

    if (q->front == q->rear)
    {
        return;
    }

    front = q->front;
    front = (front + 1) % q->maxsize;
    canTxMsg = (sCanFrameExt*)(&(q->queue[front]));
    tx_mailbox = u8can_bus_send_one_frame(*canTxMsg);
    if (tx_mailbox != CAN_NO_MB)
    {
        q->front = front;
    }
}

void CAN_Transmit_Rewrite(CAN_TypeDef* CANx, CanTxMsg* TxMessage, uint8_t transmit_mailbox)
{
    switch(transmit_mailbox)
    {
    case 0:
        while((CANx->TSR&TSR_TME0) != TSR_TME0)
        {
            if((CANx->TSR&TSR_TERR0) == TSR_TERR0)
            {
                CAN_CancelTransmit(CANx,transmit_mailbox);
            }
        }
        break;
    case 1:
        while((CANx->TSR&TSR_TME1) != TSR_TME1)
        {
            if((CANx->TSR&TSR_TERR1) == TSR_TERR1)
            {
                CAN_CancelTransmit(CANx,transmit_mailbox);
            }
        }
        break;
    case 2:
        while((CANx->TSR&TSR_TME2) != TSR_TME2)
        {
            if((CANx->TSR&TSR_TERR2) == TSR_TERR2)
            {
                CAN_CancelTransmit(CANx,transmit_mailbox);
            }
        }
        break;
    default:
        return;
    }

  if (transmit_mailbox != CAN_NO_MB)
  {
    /* Set up the Id */
    CANx->sTxMailBox[transmit_mailbox].TIR &= TMIDxR_TXRQ;
    if (TxMessage->IDE == CAN_ID_STD)
    {
      assert_param(IS_CAN_STDID(TxMessage->StdId));  
      CANx->sTxMailBox[transmit_mailbox].TIR |= ((TxMessage->StdId << 21) | TxMessage->RTR);
    }
    else
    {
      assert_param(IS_CAN_EXTID(TxMessage->ExtId));
      CANx->sTxMailBox[transmit_mailbox].TIR |= ((TxMessage->ExtId<<3) | TxMessage->IDE | 
                                               TxMessage->RTR);
    }
    

    /* Set up the DLC */
    TxMessage->DLC &= (uint8_t)0x0000000F;
    CANx->sTxMailBox[transmit_mailbox].TDTR &= (uint32_t)0xFFFFFFF0;
    CANx->sTxMailBox[transmit_mailbox].TDTR |= TxMessage->DLC;

    /* Set up the data field */
    CANx->sTxMailBox[transmit_mailbox].TDLR = (((uint32_t)TxMessage->Data[3] << 24) | 
                                             ((uint32_t)TxMessage->Data[2] << 16) |
                                             ((uint32_t)TxMessage->Data[1] << 8) | 
                                             ((uint32_t)TxMessage->Data[0]));
    CANx->sTxMailBox[transmit_mailbox].TDHR = (((uint32_t)TxMessage->Data[7] << 24) | 
                                             ((uint32_t)TxMessage->Data[6] << 16) |
                                             ((uint32_t)TxMessage->Data[5] << 8) |
                                             ((uint32_t)TxMessage->Data[4]));
    /* Request transmission */
    CANx->sTxMailBox[transmit_mailbox].TIR |= TMIDxR_TXRQ;
  }
}

void can_bus_send_one_frame(sCanFrameExt sTxMsg,u8 mailbox)
{
    CanTxMsg TxMessage;
    
    TxMessage.ExtId = (sTxMsg.extId.mac_id)|((sTxMsg.extId.func_id&0xF)<<8)|((sTxMsg.extId.seg_num&0xFF)<<12)|((sTxMsg.extId.seg_polo&0x3)<<20);
    TxMessage.IDE = CAN_ID_EXT;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = sTxMsg.data_len;
    memcpy(TxMessage.Data, sTxMsg.data, TxMessage.DLC);
    
    CAN_Transmit_Rewrite(CAN1,&TxMessage,mailbox);
}

void can_bus_reply_ack(u16 fun_id)
{
    u16 msg_type = can_recv_buff[9]|(can_recv_buff[10]<<8);
    sCanFrameExt canTxMsg;
    
    if(  ((XIAOCHE_START_NUM +20 -1) < max_guandian_count_u16)
       ||(max_guandian_count_u16 == 0) ) return; //只有最后一组车才可以发回应帧
    
    if(   (fun_id == CAN_FUNC_ID_WCS_MSG)
       && (msg_type == RECV_MSG_WCS2CAR_LOAD_CMD_TYPE) )
    {
        canTxMsg.extId.seg_polo = CAN_SEG_POLO_NONE;
        canTxMsg.extId.seg_num  = 0;
        canTxMsg.extId.func_id  = CAN_FUNC_ID_WCS_MSG;
        canTxMsg.extId.mac_id   = XIAOCHE_GROUP_NUM;
        canTxMsg.data_len = 0;
        
        can_bus_send_one_frame(canTxMsg,0);
    }
    else if(fun_id == CAN_FUNC_ID_CAR_POS)
    {
        canTxMsg.extId.seg_polo = CAN_SEG_POLO_NONE;
        canTxMsg.extId.seg_num  = 0;
        canTxMsg.extId.func_id  = CAN_FUNC_ID_CAR_POS;
        canTxMsg.extId.mac_id   = XIAOCHE_GROUP_NUM;
        canTxMsg.data_len = 0;
        
        can_bus_send_one_frame(canTxMsg,0);
    }
}

void can_bus_frame_receive(CanRxMsg rxMsg)
{
    sCanFrameExtID extID;
    
    extID.seg_polo = (rxMsg.ExtId>>20)&0x3;
    extID.seg_num  = (rxMsg.ExtId>>12)&0xFF;
    extID.func_id  = (rxMsg.ExtId>>8)&0xF;
    
    if(extID.func_id == CAN_FUNC_ID_WCS_MSG)
    {
        if(extID.seg_polo == CAN_SEG_POLO_NONE)
        {
            memcpy(can_recv_buff,rxMsg.Data,rxMsg.DLC);
            can_recv_len = rxMsg.DLC;
            can_bus_reply_ack(CAN_FUNC_ID_WCS_MSG);
            memcpy(zongxian_recv_buff, can_recv_buff, can_recv_len);
            zongxian_recv_count = can_recv_len;
            zongxian_uart_recv();
        }
        else if(extID.seg_polo == CAN_SEG_POLO_FIRST)
        {
            memcpy(can_recv_buff,rxMsg.Data,rxMsg.DLC);
            
            g_SegPolo = CAN_SEG_POLO_FIRST;
            g_SegNum = extID.seg_num;
            g_SegBytes = rxMsg.DLC;
        }
        else if(extID.seg_polo == CAN_SEG_POLO_MIDDLE)
        {
            if(   (g_SegPolo == CAN_SEG_POLO_FIRST) 
               && (extID.seg_num == (g_SegNum+1)) 
               && ((g_SegBytes+rxMsg.DLC) <= CAN_RX_BUFF_SIZE) )
            {
                memcpy(can_recv_buff+g_SegBytes,rxMsg.Data,rxMsg.DLC);
                
                g_SegNum ++;
                g_SegBytes += rxMsg.DLC;
            }
        }
        else if(extID.seg_polo == CAN_SEG_POLO_FINAL)
        {
            if(   (g_SegPolo == CAN_SEG_POLO_FIRST) 
               && (extID.seg_num == (g_SegNum+1)) 
               && ((g_SegBytes+rxMsg.DLC) <= CAN_RX_BUFF_SIZE) )
            {
                memcpy(can_recv_buff+g_SegBytes,rxMsg.Data,rxMsg.DLC);
                can_recv_len = g_SegBytes + rxMsg.DLC;
                can_bus_reply_ack(CAN_FUNC_ID_WCS_MSG);
                memcpy(zongxian_recv_buff, can_recv_buff, can_recv_len);
                zongxian_recv_count = can_recv_len;
                zongxian_uart_recv();
                
                g_SegPolo = CAN_SEG_POLO_NONE;
                g_SegNum = 0;
                g_SegBytes = 0;
            }
        }
    }
    else if(extID.func_id == CAN_FUNC_ID_CAR_POS)
    {
        memcpy(Debug_recv_buff,rxMsg.Data,rxMsg.DLC);
        can_bus_reply_ack(CAN_FUNC_ID_CAR_POS);
        recv_photoeletricity();
    }
}

u8 can_bus_send_one_test_frame(sCanFrameExt sTxMsg)
{
    CanTxMsg TxMessage;

    TxMessage.ExtId = (sTxMsg.extId.mac_id) | ((sTxMsg.extId.func_id & 0xF) << 8) | ((sTxMsg.extId.seg_num & 0xFF) << 12) | ((sTxMsg.extId.seg_polo & 0x3) << 20) | ((sTxMsg.extId.dst_id & 0x7F) << 22);
    TxMessage.IDE = CAN_ID_EXT;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = sTxMsg.data_len;
    memcpy(TxMessage.Data, sTxMsg.data, TxMessage.DLC);

    return CAN_Transmit(CAN1, &TxMessage);
}

void vcanbus_send_heart_msg(void)
{
    u8 buf[20];
    u16 sendlen = 0;

    vcan_send_send_msg(buf, sendlen, CAN_FUNC_ID_HEART_TYPE, 1);
}