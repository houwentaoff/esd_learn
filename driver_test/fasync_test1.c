/*************************************************************************
	> File Name: fasync_test1.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年06月22日 星期三 13时43分10秒
 ************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#define MAX_LEN 100

void input_handler(int num){
	char data[MAX_LEN];
	int len;
	//读取并输出STDIN_FILENO上的输入
	
	len = read(STDIN_FILENO, &data, MAX_LEN);
	data[len] = 0;
	printf("input available:%s\n", data);
}

int main(int argc, char **argv){

	int oflags;
	//启动信号驱动机制
	
	signal(SIGIO, input_handler);
	fcntl(STDIN_FILENO, F_SETOWN, getpid());
	oflags = fcntl(STDIN_FILENO, F_GETFL);
	fcntl(STDIN_FILENO, F_SETFL, oflags| FASYNC);
	
	//进入死循环,保持进程不终止,如果程序中没有这个
	//死循环会立即执行完毕
	while(1);
	return 0;
}
