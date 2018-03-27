/*************************************************************************
	> File Name: led.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年06月09日 星期二 13时37分54秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#define MISC_IOC_MAGIC 'M'
#define CMD_GET_AD _IO(MISC_IOC_MAGIC, 0x01)
int main(void)
{
	int fd;
	fd = open("/dev/adhx710_misc", O_RDWR);
	if(fd <0)
		printf("open fail\n");
	
	ioctl(fd, CMD_GET_AD);

	close(fd);

	return 0;
}
