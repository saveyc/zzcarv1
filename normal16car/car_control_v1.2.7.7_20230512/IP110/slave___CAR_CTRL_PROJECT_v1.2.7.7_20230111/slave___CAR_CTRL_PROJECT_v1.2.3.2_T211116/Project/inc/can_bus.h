#ifndef __CAN_BUS_H
#define __CAN_BUS_H
#pragma pack (1)

enum
{
    CAN_FUNC_ID_WCS_MSG = 0,
    CAN_FUNC_ID_CAR_POS = 1,
    CAN_FUNC_ID_HEART_TYPE = 4,
    CAN_FUNC_ID_SLAVER_PHOTO_CNT_TYPE = 0x0B,              //从板卡光电计数
};

enum
{
    CAN_SEG_POLO_NONE   = 0,
    CAN_SEG_POLO_FIRST  = 1,
    CAN_SEG_POLO_MIDDLE = 2,
    CAN_SEG_POLO_FINAL  = 3
};
/*
|28~22|    21~20      |    19~12     |     11~8     |    7~0      |    ExtID
|-----|  seg_polo(2)  |  seg_num(8)  |  func_id(4)  |  mac_id(8)  |
*/
typedef struct
{
    u8 seg_polo;
    u8 seg_num;
    u8 func_id;
    u8 mac_id;
    u8 dst_id;
} sCanFrameExtID;

typedef struct
{
    sCanFrameExtID extId;
    u8 data_len;
    u8 data[8];
} sCanFrameExt;

#pragma pack ()


typedef struct
{
    sCanFrameExt* queue;
    u16 front, rear, len;
    u16 maxsize;
}sCanFrameQueue;

#define CAN_PACK_DATA_LEN     8
#define CAN_MAX_FRAME_LEN     2048

#define TSR_TERR0    ((uint32_t)0x00000008)
#define TSR_TERR1    ((uint32_t)0x00000800)
#define TSR_TERR2    ((uint32_t)0x00080000)
#define TSR_TME0     ((uint32_t)0x04000000)
#define TSR_TME1     ((uint32_t)0x08000000)
#define TSR_TME2     ((uint32_t)0x10000000)
#define TMIDxR_TXRQ  ((uint32_t)0x00000001)

#define CAN_RX_BUFF_SIZE      2000//500

void can_bus_frame_receive(CanRxMsg rxMsg);
void can_bus_reply_wcs_msg(void);
void vcanbus_send_heart_msg(void);

extern sCanFrameQueue   canframequeue;
void vcan_bus_init_cansendqueue(void);
void vcan_bus_addto_cansendqueue(sCanFrameExt x);
u8 u8can_bus_send_one_frame(sCanFrameExt sTxMsg);
void vcan_send_send_msg(u8* buff, u8 send_total_len, u8 type, u8 dst);
void vcan_send_frame_process(void);

#endif