#########################################################################
# File Name: m.sh
# Author: shuixianbing
# mail: shui6666@126.com
# Created Time: 2015年11月26日 星期四 13时56分12秒
#########################################################################
#!/bin/bash
ndk-build
 cp ../libs/armeabi/*.so /home/jz/tools/adt-bundle-linux-x86_64-20131030/work/UHFrfidReaderAPIDemo/jni/
cp UhfReaderAPI.h /home/jz/tools/adt-bundle-linux-x86_64-20131030/work/UHFrfidReaderAPIDemo/jni/
adb push ../libs/armeabi/UhfReader /mnt/internal_sd
