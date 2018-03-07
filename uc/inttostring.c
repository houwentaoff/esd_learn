/*************************************************************************
	> File Name: inttostring.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年06月19日 星期一 09时28分11秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>

int main(void)
{
	int m = 25095;
	char buf[22];
	memset(buf, 0x00, sizeof(buf));
	sprintf(buf,"%d", m);
	printf("%s\n", buf);

	return 0;

}
