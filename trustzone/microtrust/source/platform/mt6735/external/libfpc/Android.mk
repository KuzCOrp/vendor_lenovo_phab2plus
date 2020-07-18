LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libFpcHalExt

LOCAL_C_INCLUDES += $(LOCAL_PATH)/hal_extension/include/

LOCAL_SRC_FILES :=  hal_extension/lib_interface.c  \
                    hal_extension/sensor.c         \
                    hal_extension/sensor_irq.c     \
                    hal_extension/sensor_ctrl.c    \
                    hal_extension/util.c

LOCAL_CFLAGS := -Wall -std=c99 -DLOG_TAG='"FpcHalExtension"'

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := libteei_fp

LOCAL_SRC_FILES := src/teei_fpc.cpp src/fpc_hal.cpp src/spi_slsi.cpp

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

#LOCAL_LDLIBS += -llog

LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_STATIC_LIBRARIES := libFpcHalExt

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := libfingerprint_tac 

LOCAL_SRC_FILES := src/fpc_lib.cpp src/teei_fpc.cpp src/fpc_hal.cpp src/spi_slsi.cpp

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

#LOCAL_LDLIBS += -llog

LOCAL_C_FLAGS := -Wall

LOCAL_CFLAGS := -DNDK_ROOT

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_STATIC_LIBRARIES := libFpcHalExt

include $(BUILD_SHARED_LIBRARY)


#include $(CLEAR_VARS)

#LOCAL_MODULE := sensor_test

#LOCAL_SRC_FILES := src/sensor_test.cpp

#LOCAL_C_INCLUDES += $(LOCAL_PATH)/include 

#LOCAL_SHARED_LIBRARIES += libteei_fp liblog

#LOCAL_LDLIBS += -llog

#include $(BUILD_EXECUTABLE)
