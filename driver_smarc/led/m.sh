#!/bin/sh 
/home/jz/tools/arm-2007q3/bin/arm-none-linux-gnueabi-gcc -o led led_test.c --static
adb push led /system/bin/
