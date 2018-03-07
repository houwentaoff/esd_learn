/*************************************************************************
	> File Name: led_test.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年11月03日 星期五 15时04分48秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MISC_IOC_MAGIC 'M'
#define LED_ON _IO(MISC_IOC_MAGIC, 0x01)
#define LED_OFF _IO(MISC_IOC_MAGIC, 0x02)
int main(int argc, char **argv){
	
	int fd = open("/dev/led", O_RDWR);
	ioctl(fd,LED_ON);

	close(fd);

	return 0;
}
