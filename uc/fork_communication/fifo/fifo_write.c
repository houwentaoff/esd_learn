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
#define FIFO_SERVER "/tmp/myfifo"

int main(int argc ,char **argv)
{
	int fd;
	char w_buf[100];
	int nwrite;

	memset(w_buf, 0, sizeof(w_buf));

	if(mkfifo(FIFO_SERVER, O_CREAT|O_EXCL|O_RDWR)<0){
		printf("cannot create fifo server\n");
	}

	fd = open(FIFO_SERVER, O_RDWR|O_NONBLOCK);
	if(fd < 0){
		perror("open");
		exit(1);
		
	}

	if(argc == 1){
		printf("please send something\n");
		exit(-1);
	}

	strcpy(w_buf, argv[1]);
	//向管道写入数据
	if(nwrite=write(fd, w_buf, 100)!=-1){
		printf("write %s to the fifo\n", w_buf);
	}

	close(fd);
	return 0;

}
