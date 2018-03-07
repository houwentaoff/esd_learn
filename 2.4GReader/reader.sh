#########################################################################
# File Name: m.sh
# Author: shuixianbing
# mail: shui6666@126.com
# Created Time: 2015年11月20日 星期五 14时56分00秒
#########################################################################
#!/bin/bash
rm -rf Reader
/home/jz/tools/arm-2007q3/bin/arm-none-linux-gnueabi-gcc -o Reader Reader.c --static
adb push Reader /mnt/internal_sd
