/*************************************************************************
	> File Name: UhfReaderAPI.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年11月26日 星期四 11时05分09秒
 ************************************************************************/
#ifndef UhfReaderAPI_H
#define UhfReaderAPI_H

#include <stdio.h>
#include "UhfReader_API.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define UART_NUM "/dev/ttymxc4"//ttymxc4
static int baudRate = 57600;
#define uint8_t unsigned char
uint8_t flagCrc = 0;
int gpio_fd,uhf_fd;


void EpcToUii(uint8_t* Uii, uint8_t* Epc, uint8_t uLenEpc);
					
void UiiToEpc(uint8_t* Uii, uint8_t* Epc, uint8_t uLenUii);

/**
 *
 *返回值 1 打开成功 0　打开失败
 *
 */
int uhfreaderopen();
/**
 *关闭超高频
 */
void uhfreaderclose();

/**
 *单步识别标签,一次只返回一个UII
 *uLenUii: Out 标签Uii的长度
 *uUii:		Out 标签UII
 */
int UhfReaderInventorySingleTag(uint8_t* uLenUii, uint8_t* uUii);

/**
 *返回标签的UII
 * uLenUii: Out 标签UII的长度
 * uUii:	Out	标签UII
 * 返回值 1成功,其它失败
 */
int UhfReaderReadInventory(uint8_t* uLenUii, uint8_t* uUii);

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
		uint8_t uCnt,uint8_t* uUii, uint8_t* uReadData);
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
		uint8_t uCnt, uint8_t* uUii,uint8_t* uWriteData);
/**
 *向标签写入多字长的数据　不指定UII
 *
 *
 *
 */
int UhfReaderBlockWriteDataToSingleTag();
/**
 *向标签写入单字长的数据　不指定UII
 *
 */
int UhfReaderWriteDataToSingleTag();
/**
 *读取标签数据不指定UII
 *
 */
int UhfReaderReadDataFromSingleTag();
#endif
