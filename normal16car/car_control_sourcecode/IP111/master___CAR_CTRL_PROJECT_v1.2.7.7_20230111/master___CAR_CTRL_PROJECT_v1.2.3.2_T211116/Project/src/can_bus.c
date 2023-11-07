#include "stm32f10x.h"
#include "stm32f107.h"
#include "main.h"

sCanFrameExt canTxMsg[2];

u8 can_resend_delay_1 = 0;
u8 can_resend_delay_2 = 0;
u8 can_resend_cnt_1 = 0;
u8 can_resend_cnt_2 = 0;
u8 can_send_flag_1 = 0;
u8 can_send_flag_2 = 0;
u8 can_send_frame_start = 0;

u8* can_send_buff = zongxian_send_buff;
u16 can_send_tot = 0;
u16 can_send_len = 0;

u32 msg_pack_id_cnt_u32 = 0;

u8 photo_send_buff[10] = { 0 };
u16 photo_send_len = 0;

#define  canframebuffsize   128
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

void vcan_send_send_msg(u8* buff, u16 send_total_len, u8 type, u8 dst)
{
    sCanFrameExt canTxMsg;
    u16 send_len = 0;

    canTxMsg.extId.func_id = type;
    canTxMsg.extId.mac_id = 1;
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

void can_send_timeout()
{
    if(can_resend_delay_1 > 0)
    {
        can_resend_delay_1--;
        if(can_resend_delay_1 == 0)
        {
            if(can_resend_cnt_1 < 1)
            {
                can_resend_cnt_1++;
                can_send_frame_start = 1;
                can_send_flag_1 = 0;
                vcan_send_send_msg(can_send_buff, can_send_tot, CAN_FUNC_ID_WCS_MSG, 2);
            }
        }
    }
    if(can_resend_delay_2 > 0)
    {
        can_resend_delay_2--;
        if(can_resend_delay_2 == 0)
        {
            if(can_resend_cnt_2 < 1)
            {
                can_resend_cnt_2++;
                can_send_flag_2 = 0;
                vcan_send_send_msg(photo_send_buff, photo_send_len, CAN_FUNC_ID_CAR_POS, 2);
            }
        }
    }
}

void vcanbus_recv_slaver_photo_cnt(u8* buff, u8 index)
{

    sphotobdcnt node;
    u16 i = 0;
    u16 num = 0;
    node.bdindex = index;
    node.photocnt = (buff[0]) | (buff[1] << 8);
    node.intervel = (buff[2]) | (buff[3] << 8);
    node.error = (buff[4]) | (buff[5] << 8);
    //    vphoto_add_to_slaver_bd_queue(node);
    slaverbdcnt.bdindex = node.bdindex;
    slaverbdcnt.photocnt = node.photocnt;
    slaverbdcnt.intervel = node.intervel;
    slaverbdcnt.error = node.error;
    slavercntflag = VALUE;

    //    vphoto_recv_slaver_photo_cmd(index, node.photocnt);
}

void can_bus_frame_receive(CanRxMsg rxMsg)
{
    u8 func_id = (rxMsg.ExtId>>8)&0xF;
    u8 dst_id = (rxMsg.ExtId >> 22) & 0x7F;
    u8 mac_id = rxMsg.ExtId & 0xFF;
    
    if(func_id == CAN_FUNC_ID_WCS_MSG)
    {
        can_resend_delay_1 = 0;
    }
    else if(func_id == CAN_FUNC_ID_CAR_POS)
    {
        can_resend_delay_2 = 0;
    }
    else if ((func_id == CAN_FUNC_ID_HEART_TYPE) && (dst_id == 1)) {
        if ((mac_id >= 2) && (mac_id <= 100)) {
            slaverheartmsg.heartcnt[mac_id - 2] = 0;
        }
    }
    else if ((func_id == CAN_FUNC_ID_SLAVER_PHOTO_CNT_TYPE) && (dst_id == 1)) {
        vcanbus_recv_slaver_photo_cnt(&(rxMsg.Data[0]), mac_id);
    }
}
void can_send_finish(u8 func_id)
{
    u16 msg_type = can_send_buff[9]|(can_send_buff[10]<<8);
    
    if(func_id == CAN_FUNC_ID_WCS_MSG)
    {
        if(msg_type == RECV_MSG_WCS2CAR_LOAD_CMD_TYPE)//四包合一
        {
            can_resend_delay_1 = 10;//10ms内没收到反馈重发
        }
        can_send_flag_1 = 0;
    }
    else if(func_id == CAN_FUNC_ID_CAR_POS)
    {
        can_resend_delay_2 = 10;//10ms内没收到反馈重发
        can_send_flag_2 = 0;
    }
}
void can_bus_send_photo_electricity(u8 level, u8 error_flag, u16 INPUT2_count)
{
    u16 i = 0;
    u8 buff[10] = { 0 };
    u16 sendlen = 0;
    //canTxMsg[1].extId.seg_polo = CAN_SEG_POLO_NONE;
    //canTxMsg[1].extId.seg_num  = 0;
    //canTxMsg[1].extId.func_id  = CAN_FUNC_ID_CAR_POS;
    //canTxMsg[1].extId.mac_id   = 1;
    //canTxMsg[1].data_len = 7;
    //canTxMsg[1].data[0] = level | (error_flag << 1);
    //canTxMsg[1].data[1] = INPUT2_count & 0xff;
    //canTxMsg[1].data[2] = (INPUT2_count >> 8) & 0xff;
    //*((u32 *) (&canTxMsg[1].data[3])) = 0;//zhuxian_speed;

    
    
    can_send_flag_2 = 1;
    can_resend_cnt_2 = 0;//重发次数清零

    photo_send_buff[0] = level | (error_flag << 1);
    photo_send_buff[1] = INPUT2_count & 0xff;
    photo_send_buff[2] = (INPUT2_count >> 8) & 0xff;
    photo_send_buff[3] = 0;
    photo_send_buff[4] = 0;
    photo_send_buff[5] = 0;
    photo_send_buff[6] = 0;
    photo_send_len = 7;

    vcan_send_send_msg(photo_send_buff, photo_send_len, CAN_FUNC_ID_CAR_POS, 2);
    can_resend_delay_2 = 10;//10ms内没收到反馈重发
    can_send_flag_2 = 0;
}

void can_bus_send_wcs_msg(u8* pbuf, u16 send_tot_len)
{

    MSG_HEAD_DATA *head = (MSG_HEAD_DATA *)pbuf;
    
    memcpy(can_send_buff, pbuf, send_tot_len);

    if(head->MSG_TYPE == RECV_MSG_WCS2CAR_LOAD_CMD_TYPE)//给四包合一数据包单独加上消息计数
    {
        msg_pack_id_cnt_u32++;
        can_send_buff[2] = msg_pack_id_cnt_u32 & 0xFF;
        can_send_buff[3] = (msg_pack_id_cnt_u32 >> 8) & 0xFF;
        can_send_buff[4] = (msg_pack_id_cnt_u32 >> 16) & 0xFF;
        can_send_buff[5] = (msg_pack_id_cnt_u32 >> 24) & 0xFF;
    }
    can_send_tot = send_tot_len;

    can_resend_cnt_1 = 0;//重发次数清零

    vcan_send_send_msg(can_send_buff, can_send_tot, CAN_FUNC_ID_WCS_MSG, 2);
    can_resend_delay_1 = 25;
}

//void can_send_frame_process()
//{
//    if(can_send_flag_1)
//    {
//        canTxMsg[0].extId.func_id  = CAN_FUNC_ID_WCS_MSG;
//        canTxMsg[0].extId.mac_id   = 1;
//        
//        if(can_send_tot <= CAN_PACK_DATA_LEN)//不需分段传输
//        {
//            canTxMsg[0].extId.seg_polo = CAN_SEG_POLO_NONE;
//            canTxMsg[0].extId.seg_num  = 0;
//            canTxMsg[0].data_len = can_send_tot;
//            memcpy(canTxMsg[0].data, can_send_buff, canTxMsg[0].data_len);
//            can_bus_send_one_frame(canTxMsg[0],0);
//            can_send_finish(CAN_FUNC_ID_WCS_MSG);
//        }
//        else//需要分段传输
//        {
//            if(can_send_frame_start)
//            {
//                canTxMsg[0].extId.seg_polo = CAN_SEG_POLO_FIRST;
//                canTxMsg[0].extId.seg_num  = 0;
//                canTxMsg[0].data_len = CAN_PACK_DATA_LEN;
//                memcpy(canTxMsg[0].data, can_send_buff, canTxMsg[0].data_len);
//                can_bus_send_one_frame(canTxMsg[0],0);
//                
//                can_send_len = CAN_PACK_DATA_LEN;
//                can_send_frame_start = 0;
//            }
//            else
//            {
//                if(can_send_len + CAN_PACK_DATA_LEN < can_send_tot)
//                {
//                    canTxMsg[0].extId.seg_polo = CAN_SEG_POLO_MIDDLE;
//                    canTxMsg[0].extId.seg_num++;
//                    canTxMsg[0].data_len = CAN_PACK_DATA_LEN;
//                    memcpy(canTxMsg[0].data, can_send_buff+can_send_len, canTxMsg[0].data_len);
//                    can_bus_send_one_frame(canTxMsg[0],0);
//                    can_send_len += CAN_PACK_DATA_LEN;
//                }
//                else
//                {
//                    canTxMsg[0].extId.seg_polo = CAN_SEG_POLO_FINAL;
//                    canTxMsg[0].extId.seg_num ++;
//                    canTxMsg[0].data_len = can_send_tot-can_send_len;
//                    memcpy(canTxMsg[0].data, can_send_buff+can_send_len, canTxMsg[0].data_len);
//                    can_bus_send_one_frame(canTxMsg[0],0);
//                    can_send_finish(CAN_FUNC_ID_WCS_MSG);
//                }
//            }
//        }
//    }
//    if(can_send_flag_2)
//    {
//        can_bus_send_one_frame(canTxMsg[1],1);
//        can_send_finish(CAN_FUNC_ID_CAR_POS);
//    }
//}