#########################################################################
# File Name: m.sh
# Author: shuixianbing
# mail: shui6666@126.com
# Created Time: 2015年11月26日 星期四 13时12分35秒
#########################################################################
#!/bin/bash
/home/jz/tools/arm-2007q3/bin/arm-none-linux-gnueabi-gcc -o uhftest UhfReaderAPI.c uhf_gpio.c -ldl --static 
adb push uhftest /mnt/internal_sd
