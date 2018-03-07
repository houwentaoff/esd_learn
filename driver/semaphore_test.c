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
char ch;
char buf[10];
int main(void)
{
	
	fd = open("/dev/hello", O_RDWR);
	while(1){
		printf("\t1. press key r to read device\n");
		printf("\t2. press key w to write device\n");
		printf("\t3. press key i to ioctl device\n");
		printf("\t4. press key q to quit device\n");

		ch = getchar();
		if('q' == ch)
			break;

		switch(ch){
			case 'r':
				read(fd, buf, 0);
				break;
			case 'w':

				write(fd, buf, 0);
				break;
			case 'i':
				
				ioctl(fd, 0, 0);
				break;
			default:
				break;
		}
		sleep(1);
	}
	close(fd);
	return 0;
}
