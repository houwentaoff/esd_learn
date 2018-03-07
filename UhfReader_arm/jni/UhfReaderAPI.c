/*************************************************************************
	> File Name: UhfReaderAPI.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年11月26日 星期四 11时05分09秒
 ************************************************************************/

#include "UhfReaderAPI.h"

void EpcToUii(uint8_t* Uii, uint8_t* Epc, uint8_t uLenEpc){

		Uii[0] = (((uLenEpc+2)/2-1)<<3)&0xf8;
			Uii[1] = 0x00;
				
				memcpy(&Uii[2], Epc, uLenEpc);
					
}


void UiiToEpc(uint8_t* Uii, uint8_t* Epc, uint8_t uLenUii){
		
		memcpy(Epc, &Uii[2], uLenUii-2);
}

/**
 *
 *返回值 1 打开成功 0　打开失败
 *
 */
int uhfreaderopen(){
	int ret = 0;

	gpio_fd = open("/dev/gpio_uhf", O_RDWR);
	if(gpio_fd < 0)
		return 0;
	//连接超高频
	ioctl(gpio_fd,1, 1);

	uhf_fd = UhfReaderConnect(UART_NUM, baudRate, flagCrc);
	if(uhf_fd <0)
		return 0;
	printf("gpio_fd=%d uhf_fd=%d\n", gpio_fd, uhf_fd);

	return 1;
}
/**
 *关闭超高频
 */
void uhfreaderclose(){
	ioctl(gpio_fd, 1, 0);
	close(gpio_fd);
	close(uhf_fd);

}
/**
 *单步识别标签,一次只返回一个UII
 *uLenUii: Out 标签Uii的长度
 *uUii:		Out 标签UII
 */
int UhfReaderInventorySingleTag(uint8_t* uLenUii, uint8_t* uUii){
		int ret = 0;

		ret = UhfInventorySingleTag(uhf_fd, uLenUii, uUii, flagCrc);
		if(ret ==1)
			printf("单步识别成功\n");
		else
			printf("单步识别失败\n");

		return ret;
}
/**
 *返回标签的UII
 * uLenUii: Out 标签UII的长度
 * uUii:	Out	标签UII
 * 返回值 1成功,其它失败
 */
int UhfReaderReadInventory(uint8_t* uLenUii, uint8_t* uUii){
		int ret = 0;

		ret = UhfReadInventory(uhf_fd, uLenUii, uUii);

		return ret;
}

/**
 *读取标签数据多字长指定EPC
 *uAccessPwd: IN 标签的访问密码 4字节
 *uBank: IN 标签的存储区 0x00 Reserved区
 *						 0x01 UII存储区
 *						 0x02 TID存储区
 *						 0x03 用户存储区
 *uPtr: IN 标签存储区的起始地址
 *uCnt: IN 数据长度,以WORD(2字节)为单位,不能为0　最大支持220
 *uUii: IN 标签的UII
 *uReadData: OUT 读取的标签数据
 *返回值:1读取标签数据成功,其它失败
 */
int UhfReaderReadDataByEPC(uint8_t* uAccessPwd,uint8_t uBank, uint8_t* uPtr,
		uint8_t uCnt,uint8_t* uUii, uint8_t* uReadData){
	int ret = 0;
	uint8_t uErrorCode[1];
	
	ret = UhfReadDataByEPC(uhf_fd, uAccessPwd, uBank, 
			uPtr, uCnt, uUii, uReadData, uErrorCode,flagCrc);
	if(ret == 1){
		printf("\n 读取标签数据指定EPC 成功\n");

	}else{
		printf("\n 读取标签数据指定EPC 失败\n");
	}
	return ret;

}
/**
 *写标签多字长的数据,指定EPC
 *uAccessPwd: In 标签的ACCESS密码4字节
 *uBank: In 标签的存储区
 *uPtr:	In 标签存储区的地址偏移量
 *uCnt:	In 写入数据长度,以字为单位
 *uUii:	In 标签的UII
 *uWriteData: In 要写入的数据内容
 *uStatus: Out 返回操作状态值 1字节
 *uWritedLen: Out 返回已经成功写入的数据长度,以字为单位
 *RuUii: Out 返回当前的标签UII号
 *
 */
int UhfReaderBlockWriteDataByEPC(uint8_t* uAccessPwd,uint8_t uBank,uint8_t* uPtr,
		uint8_t uCnt, uint8_t* uUii,uint8_t* uWriteData){
	int ret = 0;
	uint8_t uErrorCode[1];
	uint8_t uStatus[1];
	uint8_t RuUii[66];	
	uint8_t uWritedLen[1];
	ret = UhfBlockWriteDataByEPC(uhf_fd, uAccessPwd,uBank, uPtr,uCnt,uUii,
			uWriteData,uErrorCode,uStatus,uWritedLen, RuUii, flagCrc);
	if(ret == 1){
		printf("\n 写标签指定EPC成功 \n");
	}else{
		printf("\n 写标签指定EPC失败 \n");
	}

	return ret;

}
/**
 *向标签写入多字长的数据　不指定UII
 *
 *
 *
 */
int UhfReaderBlockWriteDataToSingleTag(){
	int ret = 0;
	uint8_t uUii[60];
	uint8_t uLenUii;
	uint8_t uErrorCode[1];
	uint8_t uStatus;
	uint8_t uWritedLen;
	uint8_t uPtr[2] = {0x01};
	uint8_t uPassword[4]={0x00,0x00,0x00,0x00};
	uint8_t uWriteData[14]={0x30,0x00,0xe2,0x00,0x10,0x71,0x33,0x0e,0x01,0x64,0x09,0x70,0xb1,0xc5};
	uint8_t uBank=0x01;
	uint8_t uCnt = 0x07;
	printf("-------------\n");
	ret = UhfBlockWriteDataToSingleTag(uhf_fd, uPassword, uBank, uPtr, uCnt,
			uWriteData, uUii, &uLenUii, uErrorCode, &uStatus, &uWritedLen,flagCrc);

	if(ret==1)
		printf("\n写多字节数据成功,不指定UII\n");
	return ret;


}
/**
 *向标签写入单字长的数据　不指定UII
 *
 */
int UhfReaderWriteDataToSingleTag(){
	int ret =0;
	uint8_t uUii[66];
	uint8_t uLenUii;
	uint8_t uErrorCode[1];
	uint8_t uAccessPwd[4] = {0x00,0x00,0x00,0x00};
	uint8_t uWriteData[14]={0x34,0x00,0x11,0x78,0x9a,0x10,0x11,
		0x12,0x13,0x14,0x00,0x11,0x33,0x44};
	int8_t uWriteData1[2]={0x11,0x22};
	uint8_t uPtr[2] = {0x01};
	uint8_t uCnt = 0x01;
	uint8_t uBank = 0x03;
	ret = UhfWriteDataToSingleTag(uhf_fd, uAccessPwd,uBank,uPtr,uCnt,uWriteData1,
			uUii,&uLenUii,uErrorCode,flagCrc);

	if(ret == 1)
		printf("\n单字节写标签成功\n");

	return ret;

}
/**
 *读取标签数据不指定UII
 *
 */
int UhfReaderReadDataFromSingleTag(){
	int ret = 0;
	uint8_t uReadData[55];
	uint8_t uUii[66];
	uint8_t uLenUii;
	uint8_t uErrorCode[1];
	uint8_t uAccessPwd[4] = {0x00,0x00,0x00,0x00};
	uint8_t uBank = 0x03;
	uint8_t uPtr[2]={0x01};
	uint8_t uCnt = 5;
	ret = UhfReadDataFromSingleTag(uhf_fd, uAccessPwd,uBank,uPtr,uCnt,uReadData,uUii,
			&uLenUii,uErrorCode,flagCrc);
	if(ret == 1){
		int i;
		printf("\n 读标签成功\n");
		for(i=0; i<10;i++)
			printf("0x%x ", uReadData[i]);
	}
	return ret;
}
int main(void)
{
	int ret =0;
	int i = 0;
	printf("UHF 读写器API 应用Demo\n");
	ret = uhfreaderopen();
	if(ret == 0)
		printf("超高频打开失败\n");
	else if(ret == 1)
		printf("超高频打开成功\n");
	uint8_t uLenUii;
	uint8_t uUii[66];
#if 1
	ret = UhfReaderInventorySingleTag(&uLenUii, uUii);
	if(ret == 1){
		printf("读取标签成功\n");
		printf("标签Uii长度:%d\n", uLenUii);
		printf("标签Uii数据:\n");
		for(i=0; i<uLenUii; i++)
			printf("0x%x ", uUii[i]);
	}
	else
		printf("读取标签失败\n");
#endif
	ret = UhfReaderBlockWriteDataToSingleTag();
	//ret = UhfReaderWriteDataToSingleTag();
	//	ret = UhfReaderReadDataFromSingleTag();
#if 0
		uint8_t uPassword[4] = {0x00,0x00,0x00,0x00};
		uint8_t uBank = 0x03;
		uint8_t uPtr[2] = {0x00};
		uint8_t uCnt = 0x03;
		uint8_t uUii1[6] = {0x14,0x0,0xe2,0x0,0xbb,0xbb};
		uint8_t uReadData[10];
		ret = UhfReaderReadDataByEPC(uPassword,uBank,uPtr,uCnt,uUii1,uReadData);
	if(ret == 1){
		printf("\n 读到的数据为\n");
		for(i=0; i<uCnt*2;i++)
			printf("0x%x ", uReadData[i]);
	}else
		printf("写标签失败\n");

#endif
#if 0
		uint8_t uPassword[4] = {0x00,0x00,0x00,0x00};
		uint8_t uBank = 0x03;
		uint8_t uPtr[2] = {0x00};
		uint8_t uCnt = 0x03;
		uint8_t uUii1[6] = {0x14,0x0,0xe2,0x0,0xbb,0xbb};
		uint8_t uReadData[10];
		uint8_t uWriteData[6]={0x88,0x22,0x33,0x44,0x55,0x66};
		ret = UhfReaderBlockWriteDataByEPC(uPassword,uBank,uPtr, uCnt,uUii1,
				uWriteData);
#endif
	return 0;
}
