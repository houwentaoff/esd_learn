/*************************************************************************
	> File Name: signal.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月21日 星期二 14时25分02秒
 ************************************************************************/

#include<stdio.h>
#include<signal.h>

void sig(int signo){
	printf("捕获到信号 %d\n", signo);
	sleep(1);
	//恢复signo信号的默认处理行为
	signal(signo, SIG_DFL);
}

int main(void)
{
	//进程开始捕获信号SIGINT，如果进程接收到此信号则调用sig函数
	if(signal(SIGINT, sig) == SIG_ERR){
		perror("捕获信号SIGINT失败");
	}
	//忽略信号SIGQUIT
	if(signal(SIGQUIT, SIG_IGN) == SIG_ERR){
		perror("忽略信号失败");
	}
	signal(SIGKILL, SIG_IGN);
	while(1){
		sleep(1);
		printf("程序正在运行\n");
	}

	return 0;
}
