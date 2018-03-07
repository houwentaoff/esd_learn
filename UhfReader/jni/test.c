/*************************************************************************
	> File Name: test.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年11月27日 星期五 10时09分08秒
 ************************************************************************/

#include<stdio.h>
#include <string.h>
#define uint8_t unsigned char

void EpcToUii(uint8_t* Uii, uint8_t* Epc, uint8_t uLenEpc){

	Uii[0] = (((uLenEpc+2)/2-1)<<3)&0xf8;
	Uii[1] = 0x00;
	
	memcpy(&Uii[2], Epc, uLenEpc);
	
}
void UiiToEpc(uint8_t* Uii, uint8_t* Epc, uint8_t uLenUii){
	
	memcpy(Epc, &Uii[2], uLenUii-2);
}
int main(void)
{
	uint8_t Uii[66];
	uint8_t Epc[] = {0x12,0x34,0x56,0x78,0x53,0x00,0x40,0x00,0x12,0x34,0x85,0x1a};
	//printf("uUii 的长度为 %d\n", sizeof(uUii));
	EpcToUii(Uii,Epc, sizeof(Epc));
	int i=0;
	printf("\nUii\n");
	for(i=0;i<sizeof(Epc)+2;i++)
		printf("0x%x ", Uii[i]);
	printf("\nEpc\n");
	uint8_t Epc1[66];
	UiiToEpc(Uii,Epc1,sizeof(Epc)+2);
	for(i=0; i<sizeof(Epc);i++)
		printf("0x%x ", Epc1[i]);

}
