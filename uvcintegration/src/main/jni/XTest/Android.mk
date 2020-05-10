

######################################################################
#
######################################################################
LOCAL_PATH	:= $(call my-dir)
include $(CLEAR_VARS)

######################################################################
#
######################################################################
CFLAGS := -Werror

LOCAL_C_INCLUDES := \
		$(LOCAL_PATH)/ \
		$(LOCAL_PATH)/../ \
		#$(LOCAL_PATH)/../libjpeg-turbo/libjpeg-turbo-2.0.1/ \
		#$(LOCAL_PATH)/../libjpeg-turbo/include/


LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%)
LOCAL_CFLAGS += -DANDROID_NDK
LOCAL_CFLAGS += -DLOG_NDEBUG
LOCAL_CFLAGS += -DACCESS_RAW_DESCRIPTORS
LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -ldl
LOCAL_LDLIBS += -llog
LOCAL_LDLIBS += -landroid
#LOCAL_LDLIBS += -ljnigraphics

#LOCAL_SHARED_LIBRARIES += uvc # we do not need usb1.0 here
# If we remove dependency of libusb here we can build both libusb and libuvc as static libraries
LOCAL_STATIC_LIBRARIES +=libuvc_static
LOCAL_STATIC_LIBRARIES +=libjpeg-turbo

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
		UVCReceiverDecoder.cpp \

LOCAL_MODULE    := UVCReceiverDecoder
include $(BUILD_SHARED_LIBRARY)
