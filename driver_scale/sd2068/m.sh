#!/bin/sh 
make 
adb -s 1a0381d4e6fd50d4 push rtc_sd2068.ko /data
adb -s 1a0381d4e6fd50d4 shell

