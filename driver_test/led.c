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
#include "led.h"
int main(void)
{
	int fd;
	fd = open("/dev/led", O_RDWR);
	if(fd <0)
		printf("open fail\n");
	//while(1)	

	close(fd);

	return 0;
}
