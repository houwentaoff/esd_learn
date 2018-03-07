/*************************************************************************
	> File Name: convert.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年01月13日 星期二 17时21分49秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
static void convertGreyToNV12(unsigned char *src, unsigned char *dest, int width, int height){
	int stride = 4096;
	unsigned char *bf = src;
	unsigned char *dst_y = dest;
	int i = 0;
#if 0
	for(i=0; i<height;i++){
		memcpy(dst_y, bf, width);
		bf += width;
		dst_y += stride;
	}
#endif
	memcpy(dst_y, bf, 1);
}
int main(void)
{
	unsigned char *p = "shui xian bing";
	unsigned char *q = "ddd";
	//convertGreyToNV12(p, q, 1, 1);
	memcpy(q, p, 1);
	printf("p=%s q=%s\n", p, q);
}
