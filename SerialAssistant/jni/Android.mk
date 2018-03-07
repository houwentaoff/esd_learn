LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := SerialPort
LOCAL_SRC_FILES := SerialPort.c
#LOCAL_LDLIBS := -ldl 
LOCAL_LDLIBS := -llog
#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)
