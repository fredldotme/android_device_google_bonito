LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := main.cpp
#LOCAL_SHARED_LIBRARIES := libutils \
#                          libbinder \
#                          libcutils \
#                          libaudioutils \
#                          libmedia \
#                          libhardware
LOCAL_MODULE_TAGS := optional
LOCAL_CPPFLAGS := -Wno-unused-parameter
LOCAL_MODULE := uinput-fpc-key-disable
include $(BUILD_EXECUTABLE)
