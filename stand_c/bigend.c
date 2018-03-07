/*************************************************************************
	> File Name: bigend.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月20日 星期一 15时08分10秒
 ************************************************************************/

#include<stdio.h>

int main(void)
{
	union{
		int i;
		char c;
	}u;
	u.i = 1;
	if(u.i == 1)
		printf("little\n");
	else
		printf("big\n");

	return 0;
}
