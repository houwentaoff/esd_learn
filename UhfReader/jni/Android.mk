LOCAL_PATH := $(call my-dir)
### Link libXXX.so 
include $(CLEAR_VARS)  
LOCAL_MODULE := UhfReaderCom

LOCAL_SRC_FILES := libUhfReaderCom.so 
 
include $(PREBUILT_SHARED_LIBRARY) 

include $(CLEAR_VARS)
LOCAL_MODULE    := UhfReader
LOCAL_SRC_FILES := UhfReaderAPI.c
LOCAL_SHARED_LIBRARIES += UhfReaderCom
#LOCAL_LDLIBS := -ldl 
#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)
