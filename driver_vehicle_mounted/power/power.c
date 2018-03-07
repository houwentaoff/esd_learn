/*************************************************************************
	> File Name: power.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年05月22日 星期一 09时04分29秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MISC_IOC_MAGIC 'O'
#define BD5V_PWREN_ON  _IO(MISC_IOC_MAGIC, 0x00)
#define BD5V_PWREN_OFF _IO(MISC_IOC_MAGIC, 0x01)
#define BD5V_PWREN_GET _IO(MISC_IOC_MAGIC, 0x02)


#define POWER_CMD_MAXNR 0x0c

int main(void)
{
	int fd;
	int level = 0;
	fd = open("/dev/sys_power",O_RDWR);
	if(fd < 0)
	  printf("open fail\n");

	//ioctl(fd, GPIO0_LDO_ON);
	ioctl(fd, BD5V_PWREN_ON);
	level = ioctl(fd, BD5V_PWREN_GET);
	printf("level = %d\n", level);
	//ioctl(fd, BD5V_PWREN_ON);

	
}
