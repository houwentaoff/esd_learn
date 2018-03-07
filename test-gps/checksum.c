/*************************************************************************
	> File Name: checksum.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年06月28日 星期三 13时39分31秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//GPS 较验位检查
unsigned char check(unsigned char *a){
	unsigned int i;
	unsigned char checksum=0;
	for(checksum=a[1],i=2;a[i]!='*';i++){
		checksum^=a[i];
		printf("a[%d]=%d\n", i, a[i]);
	}

	printf("result=0x%x\n", checksum);

	return checksum;

}
int main(void)
{
//	char BD_CMD[]="$BDTXA,0244883,1,0,你好*7f\r\n";
//	char BD_CMD[]="$BDCXA,1,01,0244883*43\r\n";
//	char BD_CMD[]="$CCICA,0,0*4b\r\n";
//   char BD_CMD[]="$CCTXA,244883,1,0,你好测试测试测试*52";
	char BD_CMD[] = "$CCDWA,0244883,V,1,L,,0,,,0*";
	check(BD_CMD);
	check("$BDICA,0,00*");
//	char a[15];
//	strcpy(a,"一");
//	printf("%xH %xH\n", (unsigned char)a[0], (unsigned char)a[1]);
//	check(a);
//	unsigned char  c = 0x4d^0x32;
//	printf("c=0x%x\n", c);
	return 0;
}
