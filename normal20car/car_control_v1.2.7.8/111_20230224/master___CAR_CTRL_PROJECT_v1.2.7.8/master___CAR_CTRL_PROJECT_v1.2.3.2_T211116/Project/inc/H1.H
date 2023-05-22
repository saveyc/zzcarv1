/* 车载控制系统发送小车状态信息给WCS,帧命令：0x1101*/
#define CAR_NUM 200
typedef struct{
	u8  cmd[10];
	u16 car_num;
	u16 car_index;
	u8  car_state[(u16)((CAR_NUM + 7)/8)];
}sCar2WCS_CMD_Data;

/* WCS接收到发送反馈信息至车载控制系统,帧命令：0x9101*/
typedef struct{
	u8 cmd[10];
}sCar2WCS_CMD_Data_ACK;

/*****************************************************/
/* WCS发送下料命令至车载控制系统,帧命令：0x1120*/
#define CAR_CTRL_NUM 10
typedef struct{
	u8  cmd[10];
	u8  car_ctrl_num;
	u16 car_unload_index[CAR_CTRL_NUM];
	u16 car_unload_exitno[CAR_CTRL_NUM];
        u16 car_speed;
        u16 car_startpos;
}sWCS2Car_CMD_Data;

/* 车载控制系统发送反馈信息至WCS,帧命令：0x9120*/
typedef struct{
	u8 cmd[10];
}sWCS2Car_CMD_Data_ACK;

/*****************************************************/
/* WCS发送分拣线参数信息至车载控制系统,帧命令：0X1130*/
#define CAR_NUM 200
#define EXIT_NUM 150
typedef struct{
	u8 cmd[10];
	u8 car_num;
	u8 exit_num;
	u8 exit_position[EXIT_NUM];
	u8 exit_direction[EXIT_NUM];
	u8 exit_speed[EXIT_NUM];
	u8 exit_start_time[EXIT_NUM];
	u8 exit_stop_time[EXIT_NUM];
}sWCS2Car_Para_Data;

/* 车载控制系统发送反馈信息至WCS,帧命令：0X9130*/
typedef struct{
	u8 cmd[10];
}sWCS2Car_Para_Data_ACK;




