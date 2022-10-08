
#ifndef _LOGIC_AND_CONTROL_H_
#define _LOGIC_AND_CONTROL_H_

#define EMS_communication_status 0
#define one_FM_GOOSE_link_status_A 1 //  һ�ε�ƵA��GOOSE��·״̬
#define one_FM_GOOSE_link_status_B 2 //  һ�ε�ƵB��GOOSE��·״̬
#define one_FM_Enable 3				 //  һ�ε�Ƶʹ��
#define one_FM_Disable 4			 //  һ�ε�Ƶʹ��

#define Emu_Startup 1				  //����������������
#define Emu_Stop 2					  //��������ͣ������
#define Parallel_Away_conversion_en 3 //��ת���л�ʹ��
#define Away_Parallel_conversion_en 4 //��ת���л�ʹ��
#define EMS_SET_MODE 5				  //��Ʒ����ģʽ����
#define EMS_VSG_MODE 6				  // VSG����ģʽ����
#define EMS_PQ_MODE 7				  // PQ����ģʽ����
#define BOX_35kV_ON 8				  // 35kV���߹��բ
#define BOX_35kV_OFF 9,				  // 35kV���߹��բ
#define BOX_SwitchD1_ON 10			  //���ع�D1��բ
#define BOX_SwitchD1_OFF 11			  //���ع�D1��բ
#define BOX_SwitchD2_ON 12			  //���ع�D2��բ
#define BOX_SwitchD2_OFF 13			  //���ع�D2��բ
#define EMS_PW_SETTING 14			  //�й�����
#define EMS_QW_SETTING 15			  //�޹�����
#define ONE_FM_PW_SETTING 16		  //һ�ε�Ƶ�й�����
#define ONE_FM_QW_SETTING 17		  //һ�ε�Ƶ�޹�����
typedef struct
{
	unsigned char item;	   //��Ŀ���
	unsigned char el_tag;  //  ��������
	unsigned char data[5]; //����
} YK_PARA;				   //ң�⡢ң�ز���

extern int total_pcsnum;
extern int g_flag_RecvNeed;
extern int g_flag_RecvNeed_LCD;

unsigned int countRecvFlag(int num_read);
#endif