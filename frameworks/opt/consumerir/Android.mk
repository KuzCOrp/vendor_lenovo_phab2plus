ifneq ($(MTK_IRTX_SUPPORT), yes)
ifeq ($(MTK_IR_LEARNING_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += \
      src/com/mediatek/consumerir/IConsumerIrExtraService.aidl \
	  src/com/mediatek/consumerir/ReceiveCallback.aidl \

LOCAL_MODULE:= com.mediatek.consumerir
LOCAL_MODULE_TAGS := optional

include $(BUILD_JAVA_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := com.mediatek.consumerirextra.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

endif
endif