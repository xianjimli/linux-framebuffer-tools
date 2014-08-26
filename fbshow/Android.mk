LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
				   fbshow.c

LOCAL_C_INCLUDES += \
					external/jpeg \
					external/libpng \
					external/zlib
LOCAL_STATIC_LIBRARIES:= libpng
LOCAL_SHARED_LIBRARIES:= libz libjpeg

LOCAL_MODULE:= fbshow

include $(BUILD_EXECUTABLE)
