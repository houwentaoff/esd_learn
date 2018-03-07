/*************************************************************************
	> File Name: fifo_write.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年09月01日 星期五 15时19分59秒
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#define FIFO "/tmp/myfifo"

int main(int argc ,char **argv)
{
	int fd;
	char r_buf[100];
	int nread;

	memset(r_buf, 0, sizeof(r_buf));


	fd = open(FIFO, O_RDWR|O_NONBLOCK);
	if(fd < 0){
		perror("open");
		exit(1);
		
	}

	while(1){
		memset(r_buf, 0, sizeof(r_buf));
		if((nread=read(fd, r_buf, 100))==-1){
			if(errno==EAGAIN)
			  printf("no data yet\n");
		}
		printf("read %s forom FIFO\n",r_buf);
		sleep(2);
	}

	close(fd);
	pause();
	unlink(FIFO);
	return 0;

}
