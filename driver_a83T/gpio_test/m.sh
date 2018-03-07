#########################################################################
# File Name: m.sh
# Author: shuixianbing
# mail: shui6666@126.com
# Created Time: 2016年06月12日 星期日 11时56分54秒
#########################################################################
#!/bin/bash
make
adb push misc_gpio.ko /sdcard/
