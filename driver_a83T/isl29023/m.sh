#########################################################################
# File Name: m.sh
# Author: shuixianbing
# mail: shui6666@126.com
# Created Time: 2017年03月09日 星期四 09时38分20秒
#########################################################################
#!/bin/bash
make clean
make
adb push isl29023_delay.ko /data
