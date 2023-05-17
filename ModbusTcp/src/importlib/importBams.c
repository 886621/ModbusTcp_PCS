#include "importBams.h"

#include <stdio.h>

#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "logicAndControl.h"
#define LIB_MODBMS_PATH "/usr/lib/libbams_rtu.so"
PARA_BAMS para_bams = {2, {9600, 9600}, {2, 2}, {6, 0}};
BmsData_Newest bmsdata_cur[2][18];
BmsData_Newest bmsdata_bak[2][18];
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

		// //总需求1停机
		// if(bmsdata_cur[bmsid][pcsid_bms].if_sys_fault == 1){
		// 	stopAllPcs();
		// }

		// int lcdid=0,pcs111=0,b;
		// // printf("pPara_Modtcp->pcsnum[i]");
		// if(bmsid == 0){
		// 	// lcdid = (pcsid_bms-(pcsid_bms%6))/6;
		
		// 	for(b=0;b<6;b++){
		// 		pcs111 +=pPara_Modtcp->pcsnum[i];
		// 		printf("pPara_Modtcp->pcsnum[%d]=%d\n",i,pPara_Modtcp->pcsnum[i]);
		// 		//  printf("aaa pcs111:%d pPara_Modtcp->pcsnum[%d]=%d  ",pcs111,i,pPara_Modtcp->pcsnum[i]);
		// 		 printf("--pcsid_bms:%d pcs111:%d \n",pcsid_bms,pcs111);
		// 		if(pcsid_bms > pcs111){
		// 			lcdid++;
		// 			printf("\n --lcdid:%d \n",lcdid);
		// 		}else{
		// 			pcs111 = pcsid_bms-pcs111;
		// 			// break;
		// 		}
		// 	}
		// 	// printf("----bmsid:%d--pcsid_bms:%d--lcdid:%d pcs111:%d \n",bmsid,pcsid_bms,lcdid,pcs111);
		// }else if(bmsid = 1){
		// 	// lcdid = ((pcsid_bms+g_emu_op_para.num_pcs_bms[0])-(pcsid_bms%6))/6;

		// 	for(b=0;b<6;b++){
		// 		pcs111 +=pPara_Modtcp->pcsnum[i];
		// 		// printf("bbb pcs111:%d pPara_Modtcp->pcsnum[%d]=%d",pcs111,i,pPara_Modtcp->pcsnum[i]);
		// 		 printf("--pcsid_bms:%d pcs111:%d \n",pcsid_bms,pcs111);
		// 		if(pcsid_bms > pcs111){
		// 			lcdid++;
		// 			printf("\n --lcdid:%d \n",lcdid);
		// 		}else{
		// 			pcs111 = pcsid_bms+g_emu_op_para.num_pcs_bms[0]-pcs111;
		// 			break;
		// 		}
		// 	}
		// 	// printf("----bmsid:%d--pcsid_bms:%d--lcdid:%d pcs111:%d\n",bmsid,pcsid_bms+g_emu_op_para.num_pcs_bms[0],lcdid,pcs111);
		// }

		// 电池分系统n状态为1、4、5、9、255时（停机、待机、故障、关机、调试中），向对应的PCS发停机指令
		// if(bmsdata_cur[bmsid][pcsid_bms].sys_status == 1 || bmsdata_cur[bmsid][pcsid_bms].sys_status == 4 || bmsdata_cur[bmsid][pcsid_bms].sys_status == 5 \
		// || bmsdata_cur[bmsid][pcsid_bms].sys_status == 9 || bmsdata_cur[bmsid][pcsid_bms].sys_status == 255 ){
		// 	int lcdid;
		// 	if(bmsid == 0){
		// 		lcdid = (pcsid_bms-(pcsid_bms%6))/6;
		// 	}else if(bmsid = 1){
		// 		lcdid = ((pcsid_bms+g_emu_op_para.num_pcs_bms[0])-(pcsid_bms%6))/6;
		// 		printf("------lcdid:%d \n",lcdid);
		// 	}
		// 	g_emu_action_lcd.flag_start_stop_lcd[lcdid] = 3;
		// 	// g_emu_action_lcd.action_pcs[lcdid].flag_start_stop_pcs[pcsid] = 0x55;
		// }



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
