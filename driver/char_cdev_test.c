/*************************************************************************
	> File Name: char_cdev_test.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年01月20日 星期三 10时37分44秒
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>

#define DEV_NAME "/dev/char_cdev"

int main(int argc , char **argv)
{
	int i;
	int fd = 0;
	char buf[64];

	fd = open(DEV_NAME, O_RDWR);
	if(fd < 0){
		perror("open "DEV_NAME" failed");
		exit(1);
	}

	printf("\n read orign data from device\n");
	i = read(fd, buf, 64);
	if(!i){
		perror("read "DEV_NAME" failed\n");
		exit(1);
	}
	for(i=0; i<64; i++)
		printf("0x%02x ", buf[i]);
	printf("\n");

	printf("\n write data into device\n");
	for(i=0; i<64; i++){
		buf[i] = 63-i;
	}

	i = write(fd, &buf, 64);
	if(!i){
		perror("write "DEV_NAME" failed\n");
		exit(1);
	}

	printf("\n read new data from device\n");
	i = read(fd, buf, 64);
	if(!i){
		perror("read "DEV_NAME" failed\n");
		exit(1);
	}
	for(i=0; i<64; i++)
		printf("0x%02x ", buf[i]);
	printf("\n");

	close(fd);

	return 0;
}
