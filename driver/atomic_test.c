/*************************************************************************
	> File Name: atmoic_test.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月17日 星期五 13时29分27秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
int fd;
int try_open_count =0;
int main(void)
{
	while(1){
		fd = open("/dev/hello", O_RDWR);
		if(fd > 0)
			break;
		printf("fail to open drvice\n");
		printf("trying next:%d\n", ++try_open_count);
		sleep(1);
	}
	printf("open success\n");
	while(1);
//	close(fd);
	return 0;
}
