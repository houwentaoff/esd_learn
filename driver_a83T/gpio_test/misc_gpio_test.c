/*************************************************************************
	> File Name: misc_gpio_test.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年06月12日 星期日 14时26分43秒
 ************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MISC_IOC_MAGIC 'M'
#define LED_ON  _IO(MISC_IOC_MAGIC, 0x00)
#define LED_OFF _IO(MISC_IOC_MAGIC, 0x01)

#define MISC_GPIO_MAXNR 2

int main(int argc, char **argv){

	int fd;
	fd = open("/dev/gpio_misc",O_RDWR);
	if(fd < 0)
	  printf("open fail\n");
	int i=10000;
	while(i--){
	ioctl(fd, LED_ON);
	sleep(2);
	ioctl(fd, LED_OFF);
	sleep(2);
	}

	close(fd);
}
