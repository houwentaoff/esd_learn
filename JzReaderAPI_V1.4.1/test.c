#include "JzReaderAPI.h"

int main(int argc, char **argv){
	int serial_fd;
	int ret;
//	char crc_buf[6]={0x00,0x04,0x81,0x01,0x00,0x01};
//	crc_test(crc_buf, 6);
	unsigned char TAG_CMD[4]={0x00,0x00,0x00,0x00};
	unsigned char USER_DATA[2]={0x08,0x08};
	unsigned char EPC_DATA[12]={0x00,0x01,0x55,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B};
	unsigned char pEPCRecv[19];
	unsigned char pTIDRecv[15];
//	unsigned char pUserRecv[24];
	unsigned char pUserRecv[35];

	serial_fd = XCOpen(0);
	printf("serial_fd=[%d]\n", serial_fd);	
	ret = XCPowerOn(serial_fd, 0x01);
	printf("power on ret=[%d]\n",ret);
#if 1
	ret = XCSetEpcCode(serial_fd,0x01,TAG_CMD,0x00,0x0C,EPC_DATA);
	printf("XCSetEpcCode ret=[%d]\n",ret);
	ret = XCIdentifyEpcCode(serial_fd, 0x01, 0x00, 0x00);
	printf("XCIdentifyEpcCode ret=[%d]\n", ret);
	usleep(50);
	
	ret = XCEpcCodeReport(serial_fd, pEPCRecv);
	printf("XCEpcCodeReport ret=[%d]\n",ret);
	display("EPC Code",pEPCRecv,sizeof(pEPCRecv));
#endif
	
#if 1
    ret = XCIdentifyTIDCode(serial_fd, 0x01,0x00,0x00,TAG_CMD);
	printf("XCIdentifyTIDCode ret=[%d]\n",ret);
	usleep(50);

	ret = XCTIDCodeReport(serial_fd,pTIDRecv);
	printf("XCTIDCodeReport ret = [%d]\n",ret);
	//TID编码
	display("TID Code",pTIDRecv, sizeof(pTIDRecv));
#endif
#if 1
	//写用户区数据
	ret = XCSetUserData(serial_fd, 0x01,TAG_CMD,0x00,0x02,USER_DATA);
	printf("XCSetUserData ret=[%d]\n",ret);
	ret = XCGetUserData(serial_fd, 0x01,0x00,TAG_CMD,0x00,0x02);
	printf("XCGetUserData ret=[%d]\n",ret);
	usleep(50);
	ret	= XCGetUserDataReport(serial_fd,pUserRecv);
	printf("XCGetUserDataReport ret = [%d]\n",ret);
	display("User Code",pUserRecv,sizeof(pUserRecv));
	usleep(50);
#endif
	ret	= XCPowerOff(serial_fd);
	printf("-----power off--ret=[%d]\n",ret);
	XCClose(serial_fd);
	return 0;
}
