/*************************************************************************
	> File Name: led.h
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年06月09日 星期二 13时57分22秒
 ************************************************************************/
#ifndef LED_H
#define LED_H
	
#define LED_IOCOMMAND_ON 0X1
#define LED_IOCOMMAND_OFF 0x2
#define LED_IOCOMMAND_FLASH 0x3
#define LED_IOCTL_MAGIC 'A'
#define LED_IOCTL_ON  _IO(LED_IOCTL_MAGIC, LED_IOCOMMAND_ON)
#define LED_IOCTL_OFF  _IO(LED_IOCTL_MAGIC, LED_IOCOMMAND_OFF)
#define LED_IOCTL_FLASH  _IO(LED_IOCTL_MAGIC, LED_IOCOMMAND_FLASH)
#endif

