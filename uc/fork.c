/*************************************************************************
	> File Name: fork.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月21日 星期二 14时07分03秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
int main(void)
{
	pid_t pid = fork();
	if(pid > 0){
		printf("[parent] my pid is=%d\n", getpid());
		printf("[parent] my children pid is =%d\n", pid);
	}else if(pid == 0){
		printf("[child] my pid is =%d\n", getpid());
		printf("[child] my parent pid is = %d\n", getppid());
	}else{
		perror("forck failed");
		exit(-1);
	}

	return 0;
}
