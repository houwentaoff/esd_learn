/*************************************************************************
	> File Name: Reader.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年12月01日 星期二 17时05分29秒
 ************************************************************************/

#include "Reader.h"
/**
 *初始化射频接收发送模块
 *
 */
void MRFI_gpio_init(void){
	
	ioctl(gpio_fd, RESET_EN, HIGH);
	ioctl(gpio_fd, VREG_EN, HIGH);
	ioctl(gpio_fd, HGM_EN, HIGH);
	ioctl(gpio_fd, RX_EN, HIGH);
	ioctl(gpio_fd, PAEN, HIGH);
}
int main(void)
{
	 gpio_fd = open(GPIO_DEV, O_RDWR);
	if(gpio_fd <0){
		printf("gpio 打开失败\n");
		
		return -1;
	}
	else
		printf("gpio 打开成功\n");

	
	return 0;
}
