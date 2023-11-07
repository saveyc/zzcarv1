#ifndef __FUN_H
#define __FUN_H
#pragma pack (1)
/* 车载控制系统发送小车状态信息给WCS,帧命令：0x1101*/

typedef struct
{
    uint8_t     num;
    uint8_t     statue;
    uint8_t     fangxiang;
    uint8_t     speed;
    uint16_t    dely;
    uint8_t     juli;
    uint8_t     run_en;
    uint16_t	xialiao_postion;
    uint16_t	load_postion;
    uint16_t	xiaoche_num;
    uint16_t	cur_postion;
} moto_para_struct;

typedef struct
{
    u16 tag_u16;
    u32 sequence_num_u32;
    u16 length_u16;
    u8  checksum_u8;
    u16 cmd_u16;
} sFrameHead;

typedef struct
{
    sFrameHead head_t;
    u16 car_version;
} sCar2WCS_online_Data;



/* WCS接收到发送反馈信息至车载控制系统,帧命令：0x9101*/
typedef struct
{
    u8 cmd[11];
} sCar2WCS_online_Data_ACK;
/*********************************************************
WCS发送上料命令至车载控制系统,帧命令：0x1121
*********************************************************/

#define MAX_LOAD_PLATFORM_NUM  24//48 //24

typedef struct
{
    u16 car_load_index_u16;   // index of car to load
    u8  platform_index_u8;    // index of platform to push goods to car
    u16 car_load_length_u16;  // the length of goods
    u16 car_load_offset_u16;   //distence between the center of platform and goods
    s16 car_load_delay_s16;    // unit: ms  19/3/25 add
} sCar_Load_Info;

typedef struct
{
    u8 platform_num_u8;
    sCar_Load_Info car_load_info_t[MAX_LOAD_PLATFORM_NUM];
} sLoad_info;

#define MAX_UNLOAD_CAR_NUM    24
typedef struct
{
    u16 car_unload_index_u16;
    u16 car_unload_exitno_u16;
    u8  car_unload_direction_u8;
    s16 car_unload_delay_s16;    // unit: ms
    u16 car_rotate_length_u16;
} sCar_unload_Data;

typedef struct
{
    u8 unload_car_num_u8;
    sCar_unload_Data car_unload_info_t[MAX_UNLOAD_CAR_NUM];
} sUnload_info;

typedef struct
{
    u16 star_addr;
    //u8 exit_stop;
} sIO_Info;

/* 车载控制系统发送反馈信息至WCS,帧命令：0x9120*/
typedef struct
{
    u8 cmd[11];
} sWCS2Car_CMD_Data_ACK;

/* 车载控制系统发送小车故障反馈信息至WCS,帧命令：0x1140*/
typedef struct
{
    u8 cmd[11];

} sWCS2Car_Moto_error;
/* WCS接收到发送反馈信息至车载控制系统,帧命令：0x9140*/
typedef struct
{
    u8 cmd[11];
} sCar2WCS_Moto_error_ACK;
/**************************************15530884628***************/

typedef struct
{
    u8  platform_index_u8;
    u16 a_u16;                             // 乘法因子 a  16bit， 采用定点模式，如实际值是 0.8 ,这里 a_u16 == 800;
    u16 b_u16;                             // 加法算子 b  16bit,  采用定点方式，如实际值是 100，这里 b_u16 == 100;   单位： mm
    s16 c_s16;                             // 延迟启动参数可正 可负， 16bit，                                        单位:  ms
} sLoad_platform_paras;


/* WCS发送分拣线参数信息至车载控制系统,帧命令：0X1130*/
//#define CAR_NUM 200
#define EXIT_NUM 1024
#define LOAD_NUM       MAX_LOAD_PLATFORM_NUM
typedef struct
{
    u8  cmd[11];
    u16 car_num;
    u16 exit_num;
    u16 load_num;
    u16 exit_position[EXIT_NUM];
    u8  exit_direction[EXIT_NUM];
    u8  exit_speed[EXIT_NUM];
    u8  exit_stop[EXIT_NUM];
    u16 load_position[LOAD_NUM];
    u8  load_direction[LOAD_NUM];
    u16 a_u16[LOAD_NUM];
    u16 b_u16[LOAD_NUM];
    s16 c_s16[LOAD_NUM];
    // sLoad_platform_paras sLoad_platform_paras_obj[LOAD_NUM];
} sWCS2Car_Para_Data;

/**分拣系统的开启或者关闭与WCS的同步信号包*/
typedef struct
{
    unsigned char cmd[11];
    unsigned char sys_en;
} sWCS2Car_SYS_EN_Data;


/* 车载控制系统发送反馈信息至WCS,帧命令：0X9130*/
typedef struct
{
    u8 cmd[11];
} sWCS2Car_Para_Data_ACK;

/************************************************
下料口下料控制参数 0x1135
*************************************************/
typedef struct
{
    u8 cmd[11];
    u16 car_num_u16;
    u16	moto_run_lenght;
    u16 mainline_speed;//主线运行速度(乘以1000表示，如1.35表示成1350) 19/6/25 add
} sWCS2Car_Unload_run_Data;

/************************************************
测试小车命令 0x1133
*************************************************/
typedef struct
{
    u8  cmd[11];
    u16 car_num;
    u8  direction;
    u8	speed;
    u16 length;
} sCheckCar_CMD_Data;

typedef struct
{
    u8 num;
    u8 heartcnt[100];
    u8 errnum;
    u8 errslaver[100];
}sFunslaverheart;

typedef struct
{
    u16 position;             // 头车位置
    u16 interval;             // 最小间隔
}sCar2Wcs_headcar_positon;

#pragma pack ()

typedef struct
{
    sCar2Wcs_headcar_positon* queue;
    u16 front, rear, len;
    u16 maxsize;
}sCar2Wcs_headcar_queue;

/********命令定义**********/
#define		MSG_NULL_TYPE			    0x0000
/**********主动发送命令*********/
#define		SEND_MSG_CAR2WCS_ONLINE_TYPE	    0x1101
#define		SEND_MSG_CAR2WCS_HEART_TYPE	    0x1150
#define     SEND_MSG_CAR2WCS_ERR_HEART_TYPE          0x1151
#define     RECV_SEND_MSG_CAR2WCS_HEART_TYPE_ACK     0x9150
#define     SEND_MSG_CAR2WCS_CARPOSITION_TYPE   0x1137
#define     SEND_MSG_CAR2WCS_SLAVER_HEART_TYPE      0x1138
#define     SEND_MSG_BD2PC_SLAVER_PHOTO_CNT                0x1139
/**********主动发送回复命令*********/
#define		REPLY_SEND_MSG_CAR2WCS_ONLINE_TYPE  0x9101
/**********接收命令*********/
#define		RECV_MSG_WCS2CAR_ONLINE_TYPE	    0x1120
#define		RECV_MSG_WCS2CAR_LOAD_CMD_TYPE	    0x1301
#define		RECV_MSG_WCS2CAR_PARA_TYPE	    0x1130
#define		RECV_MSG_WCS2CAR_IO_TYPE	    0x1131
#define         RECV_MSG_WCS2CAR_LOAD_OPT_RUNTIME_PARAS  0x1132
#define		RECV_MSG_WCS2CAR_FENJIAN_EN_TYPE	    0x1121
#define		RECV_MSG_WCS2CAR_UNLOAD_RUN_DATA    0x1135
#define         RECV_MSG_WCS2CAR_CHECK_CAR_TYPE     0x1133
#define         RECV_MSG_BOOT_CMD_TYPE              0x1F01
#define     RECV_MSG_SLAVER_PHOTO_CONF_TYPE         0x1142
/**********回复接收命令*********/
#define		REPLY_RECV_MSG_WCS2CAR_CMD_TYPE	    0x9120
#define		REPLY_RECV_MSG_WCS2CAR_FENJIAN_EN_TYPE	    0x9121
#define		REPLY_RECV_MSG_WCS2CAR_LOAD_CMD_TYPE	    0x9301
#define		REPLY_RECV_MSG_WCS2CAR_PARA_TYPE    0x9130
#define		REPLY_RECV_MSG_WCS2CAR_IO_TYPE    0x9131
#define         REPLY_RECV_MSG_WCS2CAR_LOAD_OPT_RUNTIME_PARAS  0x9132
#define		REPLY_RECV_MSG_WCS2CAR_UNLOAD_RUN_DATA         0x9135
#define         GUANGDIAN_CMD_TYPE      0x1240

#define         HEART_DELAY    2
#define     REPLY_RECV_MSG_SLAVER_PHOTO_CONF_TYPE         0x9142



#pragma pack (1)
typedef struct
{
    u8  MSG_TAG[2];
    u32 MSG_ID;
    u16 MSG_LENGTH;
    u8  MSG_CRC;
    u16 MSG_TYPE;
} MSG_HEAD_DATA;
#pragma pack ()

typedef struct
{
    u32  heartack_cnt;
    u8  heart_exception;
    u8  twosec;
    u8  reboot;
}sFUN_HEART_MSG;

typedef struct
{
    u16 *queue; /* 指向存储队列的数组空间 */
    u16 front, rear, len; /* 队首指针（下标），队尾指针（下标），队列长度变量 */
    u16 maxSize; /* queue数组长度 */
} MSG_SEND_QUEUE;

extern u8 udp_send_en;
extern u8 WCS_reply_timer_out;
extern u32 recive_wcs_count;
extern u16 reply_wcs_type;
extern u8 sys_moto_statue[];
extern sFUN_HEART_MSG funheart;
void InitSendMsgQueue(void);
void AddSendMsgToQueue(u16 msg);
u16 GetSendMsgFromQueue(void);
void recv_message_from_sever(u8 *point, u16 *len);
void send_message_to_sever(void);
u8 recv_msg_check(u8 *point, u16 len);
extern u32  zhuxian_speed;
extern sCar2WCS_online_Data car2WcsOnlineData;
extern sWCS2Car_Para_Data wcs2CarParaData;
extern sWCS2Car_SYS_EN_Data wcs2carSysEnData;
extern sWCS2Car_Unload_run_Data  sWCS2CarUnloadRunData;
extern void  xiaoliao_exit_process(u8 *point);


void fun_heart_init(void);
void fun_upload_heart_unconnect_process(void);
void fun_upload_heart_err_process(void);
void photo_deal_with_cnt_photo(void);
void photo_deal_with_reset_photo(void);
void vfun_slaver_heart_increase(void);
void vfun_upload_slaver_err_heart_program(void);

extern sFunslaverheart           slaverheartmsg;
extern sCar2Wcs_headcar_positon     headcarpositoon;
extern u8 Isphotovalue ;
extern u8 Isplcvalue;

#endif