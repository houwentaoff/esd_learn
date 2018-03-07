/*************************************************************************
	> File Name: wait.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年09月01日 星期五 14时43分30秒
 ************************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
int main(void)
{
	pid_t pc,pr;
	pc = fork();
	if(pc==0){
		//子进程
		printf("This is child process pid =%d\n", getpid());
		sleep(10);
	}else if(pc>0){
		//父进程
		pr = wait(NULL);
		printf("i am parent with pid %d wait child process\n",getpid());
	}
	return 0;

}
