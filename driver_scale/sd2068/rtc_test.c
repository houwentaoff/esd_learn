/*************************************************************************
	> File Name: led.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年06月09日 星期二 13时37分54秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<linux/rtc.h>

#define MISC_IOC_MAGIC 'M'
#define CMD_GET_AD _IO(MISC_IOC_MAGIC, 0x01)



#define RTC_RD_TIME _IOR('p', 0x09, struct rtc_time)

int main(void)
{
	struct rtc_time rtc_tm;
	int fd;
	char buf[7];
	fd = open("/dev/rtc0", O_RDWR);
	if(fd <0)
		printf("open fail\n");
	
	ioctl(fd, RTC_RD_TIME, &rtc_tm);

	printf("read time:%d-%d-%d %d:%d:%d\n",rtc_tm.tm_year,
				rtc_tm.tm_mon,rtc_tm.tm_mday,rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec);
	close(fd);

	return 0;
}
