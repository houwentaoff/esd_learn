/*************************************************************************
	> File Name: pipe.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年09月01日 星期五 14时57分44秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

int main(void)
{
	int pipe_fd[2];
	pid_t pid;
	char buf_r[100];
	int num;
	memset(buf_r, 0, sizeof(buf_r));

	if(pipe(pipe_fd)<0){
		printf("pipe create error\n");
		return -1;
	}else{
		printf("pipe create success\n");
	}

	if((pid=fork())==0){
		printf("chiled\n");
		close(pipe_fd[1]);
		sleep(5);

		num=read(pipe_fd[0],buf_r,100);
	
		if(num > 0)
		  printf("chiled read pipe contnet is %s\n", buf_r);
		close(pipe_fd[0]);
		exit(0);
	}else if(pid > 0){
		printf("parent\n");
		close(pipe_fd[0]);
		if(write(pipe_fd[1],"Hello",5)!=-1){
			printf("parent write pipe");
			close(pipe_fd[1]);
			waitpid(pid,NULL,0);
			exit(0);
		}
	}

	return 0;
}
