#########################################################################
# File Name: m.test.sh
# Author: shuixianbing
# mail: shui6666@126.com
# Created Time: 2016年06月12日 星期日 14时41分11秒
#########################################################################
#!/bin/bash
/home/jz/tools/arm-2007q3/bin/arm-none-linux-gnueabi-gcc -o test misc_gpio_test.c --static
adb push test /data/
