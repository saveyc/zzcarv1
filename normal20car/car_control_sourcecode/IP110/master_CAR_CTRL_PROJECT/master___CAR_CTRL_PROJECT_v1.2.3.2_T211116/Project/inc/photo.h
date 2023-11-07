#ifndef _PHOTO_H
#define _PHOTO_H

#include "main.h"

#define  INPUT_TRIG_NULL    0
#define  INPUT_TRIG_UP      1
#define  INPUT_TRIG_DOWN    2

#define  INPUT_TRIG_LOW     0
#define  INPUT_TRIG_HIGH    1

#define  RESET_PHOTO_IN   INPUT3_STATUE
#define  CNT_PHOTO_IN     INPUT2_STATUE


#define  PHOTO_RESET_FILTER    3
#define  PHOTO_CNT_FILTER      3

#define  PHOTO_INTERVAL_MIN    100
#define  PHOTO_RESET_MIN       5000

#define  PFOTO_SEND_CNT        70

#define  INVALUE               0
#define  VALUE                 1



#pragma pack (1) 


typedef struct {
    u8  input_state;
    u8  input_middle_state;
    u16 input_confirm_times;
    u8  input_trig_mode; //null,up,down
    u8  input_flag;
    u16 err_cnt;
}sInput_Info;


typedef struct {
    u16  bdindex;                    // �忨վ��
    u16  photocnt;                   // ������
    u16  intervel;                   // �����
    u16  error;                       //������Ϣ
}sphotobdcnt;

typedef  struct {
    u16  curposition;                      //ͷ��λ��
    u16  interval;                         //���֮��ļ��
    u8   photoreset;                       //ͷ���Ƿ�λ
    u16  valuecnt;                         //��紥���� ����һ����� �����ٴδ���
    u16  resetcnt;                         //��λ��紥���� һ��ʱ��
    u8   slaverrst;                        //�Ӱ忨��λ�ı�־λ 
    u8   pcslaverrst;                      //�Ӱ忨��λ��־ �ϴ���wcs��
    u8   cntlost;                          //�忨û�й���ź�
    u16  lostcount;                        //�Ӱ忨û�й���źż�ʱ
    u16  resetlost;                        //��λ�źŶ�ʧ�ƴ�
}sBD_SYS_MSG;



#pragma pack () 

typedef struct {
    sphotobdcnt* queue;
    u16 front, rear, len;
    u16 maxsize;
}sphotobdcntqueue;



extern sphotobdcnt        photobdcnt[];
extern sphotobdcntqueue   photobdcntqueue;

extern u8   photo_bd_cnt_buff[];

extern sBD_SYS_MSG                  sysmsg;
extern sBD_SYS_MSG                  phototrig;
extern sBD_SYS_MSG                  gndtrig;

extern sInput_Info  port_info[];
extern u16  input_valuecnt;

extern sphotobdcnt  slaverbdcnt;
extern u8 slavercntflag;

extern u16 upload_photoone_cnt;
extern u16 upload_phototwo_cnt;
extern u16 upload_photolast_cnt;

extern u16 photolast_num;
extern u16 photoone_num;
extern u16 phototwo_num;

extern u8  slavefirstreset;



void vphoto_init_slaver_bdcnt_queue(void);
void vphoto_add_to_slaver_bd_queue(sphotobdcnt x);
sphotobdcnt* vphoto_get_from_slaverbdqueue(void);
void vphoto_upload_slaverbdqueue(void);
void vphoto_recv_slaver_photo_cmd(u8 bdinde, u16 cnt);
void photo_trig_config(void);
void photo_deal_with_cnt_photo(void);
void photo_deal_with_reset_photo(void);

void vphoto_init_slaver_position_queue(void);
void vphoto_add_to_slaver_position_queue(sCar2Wcs_headcar_positon x);
sCar2Wcs_headcar_positon* vphoto_get_from_positionqueue(void);
void vphoto_upload_positionqueue(void);


#endif