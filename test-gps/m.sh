#########################################################################
# File Name: m.sh
# Author: shuixianbing
# mail: shui6666@126.com
# Created Time: 2017年06月28日 星期三 09时14分42秒
#########################################################################
#!/bin/bash
/home/jz/tools/arm-2007q3/bin/arm-none-linux-gnueabi-gcc -o gps-test test-gps.c --static
adb push gps-test /system/bin
