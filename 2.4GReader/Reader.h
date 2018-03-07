/*************************************************************************
	> File Name: Reader.h
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年12月01日 星期二 17时00分17秒
 ************************************************************************/

#ifndef _READER_H
#define _READER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define GPIO_DEV "/dev/gpio_cc2520"
#define HIGH 1
#define LOW	 0
#define VREG_EN 0x03
#define RESET_EN 0x04
#define CONFIG_SFD 0x05
#define CONFIG_TX 0x06
#define CONFIG_FIFOP 0x07
#define CONFIG_FIFO 0x08
#define CONFIG_CCA	0x09
#define PAEN	0x0a
#define RX_EN	0x0b
#define HGM_EN	0x0c

int gpio_fd = -1;

#endif

