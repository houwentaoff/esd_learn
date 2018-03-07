/*************************************************************************
	> File Name: crtest.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年02月04日 星期三 09时12分31秒
 ************************************************************************/

#include<stdio.h>

int main(void)
{
	int result  = 0;
	int i;
	printf("crtest starting with\n");

	for(i=0; i<100; i++){
		result += 1;
		printf("%d\n", result);
		sleep(1);
	}

	return 0;
}
