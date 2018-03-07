/*************************************************************************
	> File Name: spi.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年11月24日 星期二 10时39分04秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include "spi.h"
#include <unistd.h>


static uint8_t mode = SPI_MODE_1;//SPI通信使用全双工,设置CPOL=0,CPHA=0
static uint8_t bits = 8; //8bits读写,MSB first
static int speed = 4000000;//4M传输速度
static int delay = 0;
#define SPI_DEV "/dev/spidev3.2"

#define SPI_DEBUG 1
/**
 *同步数据传输
 *TxBuf:发送数据首地址
 *RxBuf:接收数据缓冲区
 *len:交换数据的长度
 *返回值 0 成功
 */

int spi_transfer(int fd, const uint8_t *TxBuf, uint8_t *RxBuf, int len){
	int ret;
	int i;
	struct spi_ioc_transfer tr={
		.tx_buf = (unsigned long)TxBuf,
		.rx_buf = (unsigned long)RxBuf,
		.len = len,
		.delay_usecs = delay,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if(ret < 1)
		printf("can't send spi message\n");
	else{
		#if SPI_DEBUG
		printf("send spi len:%d\n", len);
		for(i=0; i<len; i++){
			printf("0x%02x ", TxBuf[i]);
		}
		printf("\n");

		printf("spi receive len:%d\n", len);
		for(i=0; i<len;i++){
			printf("0x%02x ", RxBuf[i]);

		}
		printf("\n");
		#endif
	}
		return ret;
}
/**
 *发送数据
 *TxBuf :发送数据首地址
 *len : 发送长度
 *返回值0 成功
 */
int spi_write(int fd, uint8_t *TxBuf, int len){
	int ret = 0;
	int i;
	uint8_t rx[128];

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)TxBuf,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.delay_usecs = delay,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if(ret <0){
		printf("spi write error\n");
		return -1;
	}
	else{
	#if SPI_DEBUG
	//	printf("spi write len=%d\n", len);
		for(i=0; i<len; i++){
			printf("0x%02x ", TxBuf[i]);

		}
	#endif

	}
	return rx[len-1];
}
/***
 *接收数据
 *RxBuf:接收数据缓冲区
 *len:接收到的长度
 *返回值 0 成功
 */
int spi_read(int fd, uint8_t *RxBuf, int len){
	int ret = 0;
	int i;
	uint8_t tx[128];
	memset(tx,0xFF, sizeof(tx));
	if(len > sizeof(tx)){
		printf("spi_read len too long\n");
		return -1;
	}

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)RxBuf,
		.len = len,
		.delay_usecs = delay,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if(ret <1){
		printf("spi read error\n");
	}else{
	#if SPI_DEBUG
	int i;
	//printf("spi read len=%d\n", len);
	for(i=0; i<len; i++){
		printf("0x%02x ", RxBuf[i]);
	}
	#endif
	}
	return len;
}

int spi_init(void){
	int fd = -1;
	int ret = 0;
	fd = open(SPI_DEV, O_RDWR);
	if(fd < 0){
		printf("Can't open spidev\n'");

		return fd;
	}
	//设置SPI模式
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);//写模式
	if(ret == -1)
		printf("can't set spi mode\n");
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);//读模式
	if(ret == -1)
		printf("can't get spi mode\n'");
	//设置写每个字含多少位
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if(ret ==-1)
		printf("can't set bits per word\n");
	//设置读每个字含多少位
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if(ret == -1)
		printf("can't get bits per word\n'");
	//设置读写速度
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if(ret ==-1)
		printf("can't set max speed hz\n");
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if(ret ==-1)
		printf("can't get max speed hz");
	
	return fd;
}
#if 0
int main(void)
{

	int fd;
	int ret = -1;
	int i;
	uint8_t TxBuf[] = {0x11,0x22,0x33,0x44};
	//uint8_t *TxBuf = NULL;
	uint8_t RxBuf[4] = {0};
	fd = spi_init();
	if(fd < 0)
		printf("spi init fail\n");
	else
		printf("spi init success\n");
	spi_write(fd, TxBuf, sizeof(TxBuf));
	spi_read(fd, RxBuf, sizeof(TxBuf));
	//spi_transfer(fd, TxBuf, RxBuf, sizeof(TxBuf));
	close(fd);
	return 0;
}
#endif
