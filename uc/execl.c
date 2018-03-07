/*************************************************************************
	> File Name: execl.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月21日 星期二 14时15分23秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
int main(void)
{
	char *args[4] = {"ls", "-l", "/home", NULL};
	execvp("ls", args);
	return 0;
}
