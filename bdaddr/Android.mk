LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := op5_bdaddr_read.c

LOCAL_MODULE := bdaddr_read
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_INIT_RC := bdaddr_read.rc

LOCAL_SHARED_LIBRARIES := liblog
LOCAL_CLANG := true
include $(BUILD_EXECUTABLE)
