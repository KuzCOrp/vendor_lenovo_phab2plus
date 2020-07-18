LOCAL_PATH:= $(call my-dir)

#NOTE: for now we use static libs


############################## libhal
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:=     $(LOCAL_PATH)/hal/inc \
                    $(LOCAL_PATH)/tfa/inc \
                    $(LOCAL_PATH)/utl/inc \
                    $(LOCAL_PATH)/hal/src/lxScribo \
                    $(LOCAL_PATH)
LOCAL_SRC_FILES:= 	hal/src/NXP_I2C.c  \
			hal/src/lxScribo/lxScribo.c \
			hal/src/lxScribo/lxDummy.c  \
			hal/src/lxScribo/lxScriboSerial.c  \
			hal/src/lxScribo/lxScriboSocket.c\
	                hal/src/lxScribo/lxI2c.c \
			hal/src/lxScribo/scribosrv/i2cserver.c \
			hal/src/lxScribo/scribosrv/cmd.c
LOCAL_MODULE := libhal
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

############################### libtfa
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:=     $(LOCAL_PATH)/tfa/inc\
                        $(LOCAL_PATH)/utl/inc \
                        $(LOCAL_PATH)/hal/inc\
                        $(LOCAL_PATH)/hal/src\
                        $(LOCAL_PATH)/hal/src/lxScribo\
                        $(LOCAL_PATH)/hal/src/lxScribo/scribosrv
LOCAL_SRC_FILES := 	\
			tfa/src/initTfa9890.c\
			tfa/src/Tfa98xx.c\
			tfa/src/Tfa98xx_TextSupport.c\
			tfa/src/Tfa98API.c

LOCAL_MODULE := libtfa
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libhal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

############################### libsrv
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:=     $(LOCAL_PATH)/srv/inc\
                        $(LOCAL_PATH)/tfa/inc\
                        $(LOCAL_PATH)/utl/inc \
                        $(LOCAL_PATH)/hal/inc\
                        $(LOCAL_PATH)/hal/src\
                        $(LOCAL_PATH)/hal/src/lxScribo\
                        $(LOCAL_PATH)/srv/src/iniFile \
                        $(LOCAL_PATH)
LOCAL_SRC_FILES:= 	srv/src/nxpTfa98xx.c\
			srv/src/tfa98xxRuntime.c \
			srv/src/tfa98xxCalibration.c \
			srv/src/tfa98xxDiagnostics.c \
			srv/src/tfa98xxLiveData.c\
            srv/src/tfaOsal.c\
			srv/src/tfaContainer.c\
			srv/src/tfaContainerWrite.c\
            srv/src/tfaContUtil.c\
			srv/src/iniFile/minIni.c
LOCAL_MODULE := libsrv
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libhal libtfa
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

############################## cli app
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:=     $(LOCAL_PATH)/app/climax/src/cli\
                        $(LOCAL_PATH)/app/climax/inc\
                        $(LOCAL_PATH)/srv/inc\
                        $(LOCAL_PATH)/tfa/inc\
                        $(LOCAL_PATH)/utl/inc \
                        $(LOCAL_PATH)/hal/inc\
                        $(LOCAL_PATH)/hal/src
LOCAL_SRC_FILES:= 	app/climax/src/climax.c \
			app/climax/src/cliCommands.c \
			app/climax/src/cli/cmdline.c
LOCAL_MODULE := climax_hostSW
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libsrv libtfa libhal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_EXECUTABLE)




############################## tfa9890_test
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:=     $(LOCAL_PATH)/srv/inc\
                        $(LOCAL_PATH)/tfa/inc\
                        $(LOCAL_PATH)/hal/inc\
                        $(LOCAL_PATH)/app/exTfa98xx/inc \
                        $(LOCAL_PATH)/hal/src
LOCAL_SRC_FILES:=     app/exTfa98xx/src/main_container.c
LOCAL_CFLAGS:=-DAndroid
LOCAL_MODULE := tfa9890_test
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libsrv libtfa libhal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_EXECUTABLE)

############################## libtfa9890
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:=     $(LOCAL_PATH)/srv/inc\
                        $(LOCAL_PATH)/tfa/inc\
                        $(LOCAL_PATH)/hal/inc\
                        $(LOCAL_PATH)/app/exTfa98xx/inc \
                        $(LOCAL_PATH)/hal/src
LOCAL_SRC_FILES:=     app/exTfa98xx/src/main_container.c
LOCAL_CFLAGS:=-DAndroid
LOCAL_MODULE := libtfa9890
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libsrv libtfa libhal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
