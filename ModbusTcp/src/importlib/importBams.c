#include "importBams.h"

#include <stdio.h>

#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "logicAndControl.h"
#define LIB_MODBMS_PATH "/usr/local/lib/libbams_rtu.so"
PARA_BAMS para_bams = {2, {9600, 9600}, {2, 2}, {6, 0}};
BmsData_Newest bmsdata_cur[2][18];
BmsData_Newest bmsdata_bak[2][18];
unsigned char bms_ov_status[6]={0,0,0,0,0,0};
int checkBmsForStart(int sn)
{
	unsigned short status_bms;
	float soc_ave, soc_ave_p20, soc_ave_n20; // = g_emu_op_para.soc_ave;
	float soc;


	status_bms = bmsdata_cur[0][sn].sys_status;

	if (status_bms & ((1 << BMS_ST_WORKING) == 0))
		return 1;
	soc_ave = (float)g_emu_op_para.soc_ave;

	if (sn <= g_emu_op_para.num_pcs_bms[0])
	{
		soc = (float)bmsdata_cur[0][sn].soc;
	}
	else
	{
		soc = (float)bmsdata_cur[1][sn - g_emu_op_para.num_pcs_bms[0] + 1].soc;
	}
	soc_ave_p20 = (soc_ave * 6) / 5;
	soc_ave_n20 = (soc_ave * 4) / 5;

	printf("checkBmsForStart soc_ave=%f soc_ave_p20=%f soc_ave_n20=%f\n ", soc_ave, soc_ave_p20, soc_ave_n20);
	if ((soc >= soc_ave_p20) || (soc <= soc_ave_n20))
	{
		return 2;
	}
	return 0;
}
int countPcsNum_Bms(unsigned int flag_recv)
{
	int i;
	int num_pcs = 0;

	for (i = 0; i < 18; i++)
	{
		if ((flag_recv & (1 << i)) != 0)
			num_pcs++;
	}
	return num_pcs;
}

int lcdPcsCount(unsigned char bmsid,unsigned char pcsid_bms,unsigned char *pLcdid,unsigned char *pLcd_pcs_id){
	int i;
	unsigned char lcdid=0,lcd_pcs_id=0;
	if(bmsid == 0){
		for(i=0;i<MAX_PCS_NUM;i++){
			lcd_pcs_id +=pPara_Modtcp->pcsnum[i];
			if((pcsid_bms+1) > lcd_pcs_id){
				lcdid++;
			}else{
				lcd_pcs_id = (pcsid_bms+1)-(lcd_pcs_id-pPara_Modtcp->pcsnum[i]);
				break;
			}
		}
	}else if(bmsid == 1){
		unsigned char pcsid_bms1 = (pcsid_bms+1) + 14;
		for(i=0;i<MAX_PCS_NUM;i++){
			lcd_pcs_id +=pPara_Modtcp->pcsnum[i];
			if(pcsid_bms1 > lcd_pcs_id){
				lcdid++;
			}else{
				lcd_pcs_id = pcsid_bms1-(lcd_pcs_id-pPara_Modtcp->pcsnum[i]);
				break;
			}
		}
	}
	*pLcdid=lcdid;
	*pLcd_pcs_id=lcd_pcs_id-1;
	return 0;		
}

int recvfromBams(unsigned char pcsid_bms, unsigned char type, void *pdata)
{
	int i;
	switch (type)
	{
		case _ALL_:
		{
			static unsigned int flag_recv_bms[] = {0, 0};
			int num_pcs1, num_pcs2, num_pcs;
			BmsData temp = *(BmsData *)pdata;
			unsigned char bmsid = temp.bmsid;
			

			printf("xxxLCD模块收到BAMS传来的所有数据！bmsid=%d pcsid=%d %d \n",temp.bmsid, pcsid_bms, temp.pcsid_bms);
			bmsdata_cur[bmsid][pcsid_bms].mx_cpw = temp.buf_data[BMS_MX_CPW * 2] * 256 + temp.buf_data[BMS_MX_CPW * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].mx_dpw = temp.buf_data[BMS_MX_DPW * 2] * 256 + temp.buf_data[BMS_MX_DPW * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].heartbeat = temp.buf_data[BMS_CONN_HEARTBEAT * 2] * 256 + temp.buf_data[BMS_CONN_HEARTBEAT * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].main_vol = temp.buf_data[BMS_MAIN_VOLTAGE * 2] * 256 + temp.buf_data[BMS_MAIN_VOLTAGE * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].mx_ccur = temp.buf_data[BMS_MX_CCURRENT * 2] * 256 + temp.buf_data[BMS_MX_CCURRENT * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].mx_dccur = temp.buf_data[BMS_MX_DCURRENT * 2] * 256 + temp.buf_data[BMS_MX_DCURRENT * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].sum_cur = temp.buf_data[BMS_SUM_CURRENT * 2] * 256 + temp.buf_data[BMS_SUM_CURRENT * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].soc = temp.buf_data[BMS_SOC * 2] * 256 + temp.buf_data[BMS_SOC * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].remain_ch_cap = temp.buf_data[BMS_remaining_charging_capacity * 2] * 256 + temp.buf_data[BMS_remaining_charging_capacity * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].remain_dch_cap = temp.buf_data[BMS_remaining_discharging_capacity * 2] * 256 + temp.buf_data[BMS_remaining_discharging_capacity * 2 + 1];

			bmsdata_cur[bmsid][pcsid_bms].single_mx_vol = temp.buf_data[BMS_single_MX_voltage * 2] * 256 + temp.buf_data[BMS_single_MX_voltage * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].single_mi_vol = temp.buf_data[BMS_single_MI_voltage * 2] * 256 + temp.buf_data[BMS_single_MI_voltage * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].sys_status = temp.buf_data[BMS_SYS_STATUS * 2] * 256 + temp.buf_data[BMS_SYS_STATUS * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].sys_need = temp.buf_data[BMS_SYS_NEED * 2] * 256 + temp.buf_data[BMS_SYS_NEED * 2 + 1];
			bmsdata_cur[bmsid][pcsid_bms].if_sys_fault = temp.buf_data[BMS_FAULT_STATUS * 2] * 256 + temp.buf_data[BMS_FAULT_STATUS * 2 + 1];
			flag_recv_bms[bmsid] |= (1 << pcsid_bms);

			num_pcs1 = countPcsNum_Bms(flag_recv_bms[0]);
			num_pcs2 = countPcsNum_Bms(flag_recv_bms[1]);
				
			printf("33223322 flag_recv_bms[0]=%x  flag_recv_bms[1]=%x num_pcs1=%d num_pcs2=%d\n", flag_recv_bms[0], flag_recv_bms[1], num_pcs1, num_pcs2);
			printf("bmsdata_cur[bmsid=%d][pcsid_bms=%d].soc=%d\n",bmsid,pcsid_bms,bmsdata_cur[bmsid][pcsid_bms].soc);

		

			if ((num_pcs1 + num_pcs2) >= total_pcsnum)
			{

				printf("33443344 flag_recv_bms[0]=%x  flag_recv_bms[1]=%x num_pcs1=%d num_pcs2=%d\n", flag_recv_bms[0], flag_recv_bms[1], num_pcs1, num_pcs2);
				unsigned int temp = 0;
				num_pcs = num_pcs1 + num_pcs2;
				for (i = 0; i < num_pcs1; i++)
				{
					temp += bmsdata_cur[0][i].soc;
				}
				for (i = 0; i < num_pcs2; i++)
				{
					temp += bmsdata_cur[1][i].soc;
				}
				temp /= num_pcs;
				g_emu_op_para.soc_ave = temp;
				g_emu_op_para.num_pcs_bms[0] = num_pcs1;
				g_emu_op_para.num_pcs_bms[1] = num_pcs2;
				memcpy((unsigned char*)bmsdata_bak,(unsigned char*)bmsdata_cur,sizeof(BmsData_Newest)*36);
				g_emu_op_para.flag_soc_bak=1;
				printf("g_emu_op_para.soc_ave=%d  num_pcs_bms1=%d  num_pcs_bms2=%d\n", g_emu_op_para.soc_ave, g_emu_op_para.num_pcs_bms[0], g_emu_op_para.num_pcs_bms[1]);
				
				printf("emu中每个pcs的soc打印:\n");
				printf_pcs_soc();
				
				flag_recv_bms[0] = 0;
				flag_recv_bms[1] = 0;
			}

			unsigned char lcdid=0,lcd_pcs_id=0;
			 lcdPcsCount(bmsid,pcsid_bms,&lcdid,&lcd_pcs_id);
			/*
				1.单体正常充放电截止电压区间 2.90V~3.55V，PCS 检测到电池分系统单体最高电压达到3.6V，PCS 应停机或封脉冲；电池分系统单体,最高电压达到 3.63V，PCS 应关机；
				2.单体正常充放电截止电压区间 2.90V~3.55V，PCS 检测到电池分系统单体最低电压达到2.85V，PCS 应停机或封脉冲；电池分系统单体,最低电压达到 2.75V，PCS 应关机；
			*/
			float single_mx_vol =  (float)bmsdata_cur[bmsid][pcsid_bms].single_mx_vol/1000;
			float single_mi_vol =  (float)bmsdata_cur[bmsid][pcsid_bms].single_mi_vol/1000;
			printf("tttaaa bms 最高单体电压：%f  最低单体电压：%f lcdid:%d pcsid:%d flag_start_stop:%d\n",single_mx_vol,single_mi_vol,lcdid,lcd_pcs_id,g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[lcd_pcs_id]);
			printf("tttbbb 配置文件 最高单体电压：%f  最低单体电压：%f \n",pPara_Modtcp->Maximum_individual_voltage,pPara_Modtcp->Minimum_individual_voltage);
			// if( single_mx_vol>= 3.55 || single_mi_vol <= 2.9){
			if( single_mx_vol>= pPara_Modtcp->Maximum_individual_voltage || single_mi_vol <= pPara_Modtcp->Minimum_individual_voltage){
					printf("tttaaa1111 bms 最高单体电压：%f  最低单体电压：%f lcdid:%d pcsid:%d flag_start_stop:%d\n",single_mx_vol,single_mi_vol,lcdid,lcd_pcs_id,g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[lcd_pcs_id]);
					if(((bms_ov_status[lcdid] & (1 << lcd_pcs_id)) == 0) && g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[lcd_pcs_id] == 1){
						bms_ov_status[lcdid] |= (1 << lcd_pcs_id);
						printf("已经置1：%d %d %d %d bms_ov_status[%d]%d \n",bmsid,pcsid_bms,lcdid,lcd_pcs_id,lcdid,bms_ov_status[lcdid]);
					}
			}

		/*
		// unsigned char lcdid=0,lcd_pcs_id=0, lcdid1=0,lcd_pcs_id1=0;
			lcdPcsCount(bmsid,pcsid_bms,&lcdid,&lcd_pcs_id);
			printf("bamsaa bmsid:%d pcsid_bms:%d lcdid：%d %d g_emu_status_lcd.status_pcs[%d].flag_start_stop[%d]:%d \n",bmsid,pcsid_bms+1,lcdid,lcd_pcs_id,lcdid,lcd_pcs_id,g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[lcd_pcs_id]);
			int all_stop_flag=0;  //1： 全部设备停机   0：还有设备不是停机

			// 总需求1停机
			if(bmsdata_cur[bmsid][pcsid_bms].if_sys_fault == 1){
				for(i=0;i<MAX_LCD_NUM;i++){
					for(j=0;j<MAX_PCS_NUM;j++){
						if(g_emu_status_lcd.status_pcs[i].flag_start_stop[j] == 0){
							all_stop_flag=1;
						}
						else{
							all_stop_flag=0;
							break;
						}
					}	
				}
				if(all_stop_flag==1)
					return 0;

				printf("bms发停机指令 总需求1停机，对所有PCS发停机指令;\n");
				stopAllPcs();
			}

			//电池分系统n通讯心跳不更新超过10s（通讯中断），对所有PCS发停机指令;
			if(bmsdata_cur[bmsid][pcsid_bms].heartbeat == bmsdata_bak[bmsid][pcsid_bms].heartbeat){
				if(bams_heartbeat_timer_flag[bmsid][pcsid_bms] == 0){
					bams_heartbeat_timer[bmsid][pcsid_bms] = 900;
					bams_heartbeat_timer_flag[bmsid][pcsid_bms] = 1;
				}
				if(bams_heartbeat_timer[bmsid][pcsid_bms] <= 0){
					for(i=0;i<MAX_LCD_NUM;i++){
						for(j=0;j<MAX_PCS_NUM;j++){
							if(g_emu_status_lcd.status_pcs[i].flag_start_stop[j] == 0){
								all_stop_flag=1;
							}
							else{
								all_stop_flag=0;
								break;
							}	
						}
					}
					if(all_stop_flag==1)
							return 0;

						printf("bms发停机指令 电池分系统n通讯心跳不更新超过10s（通讯中断），对所有PCS发停机指令;\n");
						stopAllPcs();
					}
			}else{
					bams_heartbeat_timer[bmsid][pcsid_bms] = 0;
					bams_heartbeat_timer_flag[bmsid][pcsid_bms] = 0;
			}

			//电池分系统n需求为0时（PCS禁止充放电）
			if(bmsdata_cur[bmsid][pcsid_bms].sys_need == 0){
				unsigned char a,b;
				for(a=0;a<2;a++){
					for(b=0;b<18;b++){
						if(bmsdata_cur[a][b].sys_need == 1 || bmsdata_cur[a][b].sys_need == 2){
							lcdPcsCount(a,b,&lcdid1,&lcd_pcs_id1);
							if(g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[lcd_pcs_id] == 0)
								return 0;
							printf("bms发停机指令 电池分系统n需求为0时（PCS禁止充放电）\n");
							g_emu_action_lcd.flag_start_stop_lcd[lcdid1] = 3;
							g_emu_action_lcd.action_pcs[lcdid1].flag_start_stop_pcs[lcd_pcs_id1] = 0xaa;
						}
					}
				}
			}

			
			
			// 电池分系统n状态为1、4、5、9、255时（停机、待机、故障、关机、调试中），向对应的PCS发停机指令
			if(bmsdata_cur[bmsid][pcsid_bms].sys_status == 1 || bmsdata_cur[bmsid][pcsid_bms].sys_status == 4 || bmsdata_cur[bmsid][pcsid_bms].sys_status == 5 \
			|| bmsdata_cur[bmsid][pcsid_bms].sys_status == 9 || bmsdata_cur[bmsid][pcsid_bms].sys_status == 255 ){
				if(g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[lcd_pcs_id] == 0)
					return 0;
				
				printf("bms发停机指令 电池分系统n状态为1、4、5、9、255时（停机、待机、故障、关机、调试中），向对应的PCS发停机指令\n");
				g_emu_action_lcd.flag_start_stop_lcd[lcdid] = 3;
				g_emu_action_lcd.action_pcs[lcdid].flag_start_stop_pcs[lcd_pcs_id] = 0xaa;			
			}

			
			//1.单体正常充放电截止电压区间 2.90V~3.55V，PCS 检测到电池分系统单体最高电压达到3.6V，PCS 应停机或封脉冲；电池分系统单体,最高电压达到 3.63V，PCS 应关机；
			//2.单体正常充放电截止电压区间 2.90V~3.55V，PCS 检测到电池分系统单体最低电压达到2.85V，PCS 应停机或封脉冲；电池分系统单体,最低电压达到 2.75V，PCS 应关机；
			
			float single_mx_vol =  (float)bmsdata_cur[bmsid][pcsid_bms].single_mx_vol/1000;
			float single_mi_vol =  (float)bmsdata_cur[bmsid][pcsid_bms].single_mi_vol/1000;

			printf("bms 最高单体电压：%f  最低单体电压：%f g_emu_status_lcd.status_pcs[%d].flag_start_stop[%d]:%d \n",single_mx_vol,single_mi_vol,lcdid,lcd_pcs_id,g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[lcd_pcs_id]);
			if( single_mx_vol>= 3.3 || single_mi_vol <= 2.9){
				

				printf("bamsaa11 bmsid:%d pcsid_bms:%d lcdid：%d %d g_emu_status_lcd.status_pcs[%d].flag_start_stop[%d]:%d \n",bmsid,pcsid_bms+1,lcdid,lcd_pcs_id,lcdid,lcd_pcs_id,g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[lcd_pcs_id]);
				if(g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[lcd_pcs_id] == 0)
					return 0;
				
				printf("bms发停机指令 单体正常充放电截止电压区间 向对应的PCS发停机指令 lcdid：%d  lcd_pcs_id:%d \n",lcdid,lcd_pcs_id);
				g_emu_action_lcd.flag_start_stop_lcd[lcdid] = 3;
				g_emu_action_lcd.action_pcs[lcdid].flag_start_stop_pcs[lcd_pcs_id] = 0xaa;		
			}
		*/
		}
		break;
		case _SOC_:
		{
			short soc = *(short *)pdata;
			printf("LCD模块收到BAMS传来的soc数据！pcsid_bms=%d soc=%d\n", pcsid_bms, soc);
		}
		break;
		default:
			break;
	}
	return 0;	
}


void bams_Init(void)
{
	void *handle;
	char *error;
	typedef int (*init_fun)(void *);
	typedef int (*outBmsData2Other)(unsigned char, unsigned char, void *); //输出数据
	typedef int (*indata_fun)(unsigned char type, outBmsData2Other pfun);  //命令处理函数指针
	printf("xxLCD模块动态调用BAMS模块！\n");
	init_fun my_func = (void *)0;
	indata_fun my_func_putin = (void *)0;

	//打开动态链接库
	handle = dlopen(LIB_MODBMS_PATH, RTLD_LAZY);
	if (!handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror();

	*(void **)(&my_func) = dlsym(handle, "bams_main");
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	printf("1LCD模块动态调用BAMS模块！\n");
	*(void **)(&my_func_putin) = dlsym(handle, "SubscribeBamsData");
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);

		exit(EXIT_FAILURE);
	}
	printf("2LCD模块动态调用BAMS模块！\n");
	my_func((void *)&para_bams);
	printf("LCD模块订阅BAMS数据\n");
	my_func_putin(_ALL_, recvfromBams);
	my_func_putin(_SOC_, recvfromBams);

}
