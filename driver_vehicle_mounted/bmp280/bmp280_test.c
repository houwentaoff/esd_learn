/*************************************************************************
	> File Name: bmp280_test.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年06月03日 星期六 11时14分18秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MISC_IOC_MAGIC 'B'
#define GET_TEMP_DATA  _IO(MISC_IOC_MAGIC, 0x00)
#define GET_PRESS_DATA _IO(MISC_IOC_MAGIC, 0x01)

int main(void)
{
	int fd;
	int temp=0;
	int press=0;
	char str[10];
	fd = open("/dev/bmp280",O_RDWR);
	if(fd < 0)
	  printf("open fail\n");

	ioctl(fd, GET_TEMP_DATA, &temp);
	sprintf(str,"%0.2f", temp/100.0);
	printf("the temp is %s C\n", str);

	ioctl(fd, GET_PRESS_DATA,&press);
	sprintf(str ,"%0.2f", press/1000.0);
	printf("press is %s Kpa\n", str);
	close(fd);
}
