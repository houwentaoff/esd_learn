#ifndef _JZREADER_H
#define _JZREADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>

#define ERROR_NODATA_RETURN 0
#define FUCCESS_RETURN	1
#define ERROR_FD_VALUE -1 //返回文件描述符错误
#define ERROR_USERDATA_LEN -2
#define ERROR_USERDATA_ADDRESS -3
#define ERROR_RECEIVEDATA_LEN -4 //接收数据长度错误
#define ERROR_SENDDATA_FAIL -5 //接收数据错
#define ERROR_READWRITE_FAIL -7
#define ERROR_IDENTIFY_FAIL -8//第n号天线没有连接
#define ERROR_OTHER_FAIL -20//其它错误


#define MAX_REC_NUM 256
#define MIN_COMM_NUM 6
extern int  XCOpen(int com_port);
extern int XCClose(int fd);
extern int  XCPowerOn(int fd, unsigned char ucAntenna);
extern int  XCPowerOff(int fd);
extern int  XCIdentifyEpcCode(int fd, unsigned char ucAntenna,
									 unsigned char ucScanTimes,
									 unsigned char isDoAffirm);
extern int  XCEpcCodeReport(int fd, unsigned char *pEpcCode);
extern int  XCSetEpcCode(int fd, unsigned char ucAntenna,
							     unsigned char *pTagPWD,
								 unsigned char ucStartAdd,
								 unsigned char ucLength,
								 unsigned char *pEpcValue);
extern int  com_frame_rev(int fd, unsigned char *p_out, int buffer_size);

extern int  XCIdentifyTIDCode(int fd, unsigned char ucAntenna,
									  unsigned char ucScanTImes,
									  unsigned char isDoAffirm,
									  unsigned char *pTagPWD);
extern int  XCTIDCodeReport(int fd, unsigned char *pTIDCode);
extern int  XCGetUserData(int fd, unsigned char ucAntenna,
								  unsigned char isDoAffirm,
							     unsigned char *pTagPWD,
								 unsigned char ucStartAdd,
								  unsigned char ucLength);
						
extern int  XCGetUserDataReport(int fd, unsigned char *pValue);
extern int  XCSetUserData(int fd, unsigned char ucAntenna,
							      unsigned char *pTagPWD,
								  unsigned char ucStartAdd,
								  unsigned char ucLength,
								  unsigned char *pValue);
extern int XCSysQuery(int fd, int iInfoType,
							  unsigned char size,
							  unsigned char *pData);
extern int XCSysFastSet(int fd, int iInfoType,
							    int size,
								unsigned char *Param);
extern int XCSetDataLocked(int fd, unsigned char ucAntenna,
								   unsigned char *pTagPWD,
								   unsigned char ucLocked,
								   unsigned char ucDIstrict);
extern int XCSetVisitPWD(int fd, unsigned char ucAntenna,
						         unsigned char *pTagPWD,
								 unsigned char *pNewTagPWD);
extern int XCSetDestroyPWD(int fd, unsigned char ucAntenna,
								   unsigned char *pTagPWD,
								   unsigned char *pNewTagPWD);
extern int XCDestroyTag(int fd, unsigned char ucAntenna, 
								unsigned char *pDestroyPWD);
extern int XCDataReturn(int fd, unsigned char Command,
								unsigned char DataType);
void display(char *Tag, unsigned char *pbuf, int size);
extern int XCIsError(unsigned char *RevData);
#endif
