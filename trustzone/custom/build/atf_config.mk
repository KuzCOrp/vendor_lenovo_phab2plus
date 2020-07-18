include $(TRUSTZONE_CUSTOM_BUILD_PATH)/common_config.mk

##
# global setting
##
ifeq ($(TARGET_BUILD_VARIANT),eng)
  ATF_INSTALL_MODE ?= Debug
else
  ATF_INSTALL_MODE ?= Debug
endif


### CUSTOMIZTION FILES ###
PART_DEFAULT_MEMADDR := 0xFFFFFFFF


### ATF SETTING ###
ifeq ($(ATF_INSTALL_MODE),Debug)
  ATF_INSTALL_MODE_LC := debug
  ATF_DEBUG_ENABLE := 1
else
  ATF_INSTALL_MODE_LC := release
  ATF_DEBUG_ENABLE := 0
endif
ifndef MTK_ATF_VERSION
  $(error MTK_ATF_VERSION is not defined)
endif
ATF_BUILD_PATH := vendor/mediatek/proprietary/trustzone/atf/$(MTK_ATF_VERSION)
export ATF_ADDITIONAL_DEPENDENCIES := $(abspath $(TRUSTZONE_PROJECT_MAKEFILE) $(TRUSTZONE_CUSTOM_BUILD_PATH)/common_config.mk $(TRUSTZONE_CUSTOM_BUILD_PATH)/atf_config.mk)
ATF_RAW_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/ATF_OBJ/$(ATF_INSTALL_MODE_LC)/bl31.bin
ATF_TEMP_PADDING_FILE := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_atf_$(ATF_INSTALL_MODE_LC)_pad.txt
ATF_TEMP_CFG_FILE := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/img_hdr_atf.cfg
ATF_SIGNED_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_atf_$(ATF_INSTALL_MODE_LC)_signed.img
ATF_PADDING_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_atf_$(ATF_INSTALL_MODE_LC)_pad.img
ATF_COMP_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_atf.img

ifeq ($(ATF_CROSS_COMPILE),)
#ATF_CROSS_COMPILE := $(abspath $(TARGET_TOOLS_PREFIX))
endif
ATF_GLOBAL_MAKE_OPTION := $(if $(ATF_CROSS_COMPILE),CROSS_COMPILE=$(ATF_CROSS_COMPILE)) BUILD_BASE=$(abspath $(TRUSTZONE_IMAGE_OUTPUT_PATH)/ATF_OBJ) DEBUG=$(ATF_DEBUG_ENABLE) PLAT=$(ARCH_MTK_PLATFORM) SECURE_OS=$(TRUSTZONE_IMPL) MACH_TYPE=$(MTK_MACH_TYPE)
ifneq ($(TRUSTZONE_ROOT_DIR),)
  ATF_GLOBAL_MAKE_OPTION += ROOTDIR=$(TRUSTZONE_ROOT_DIR)
endif

$(ATF_RAW_IMAGE_NAME): FORCE
	@echo ATF build: $@
	$(hide) mkdir -p $(dir $@)
	$(MAKE) -C $(ATF_BUILD_PATH) $(ATF_GLOBAL_MAKE_OPTION) all

$(ATF_TEMP_PADDING_FILE): ALIGNMENT=512
$(ATF_TEMP_PADDING_FILE): MKIMAGE_HDR_SIZE=512
$(ATF_TEMP_PADDING_FILE): RSA_SIGN_HDR_SIZE=576
$(ATF_TEMP_PADDING_FILE): $(ATF_RAW_IMAGE_NAME) $(ATF_ADDITIONAL_DEPENDENCIES)
	@echo ATF build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) FILE_SIZE=$$(($$(wc -c < "$(ATF_RAW_IMAGE_NAME)")+$(MKIMAGE_HDR_SIZE)+$(RSA_SIGN_HDR_SIZE)));\
	REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
	if [ $${REMAINDER} -ne 0 ]; then dd if=/dev/zero of=$@ bs=$$(($(ALIGNMENT)-$${REMAINDER})) count=1; else touch $@; fi

$(ATF_TEMP_CFG_FILE): $(ATF_ADDITIONAL_DEPENDENCIES)
	@echo ATF build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) rm -f $@
	@echo "LOAD_MODE = 0" > $@
	@echo "NAME = atf" >> $@
	@echo "LOAD_ADDR =" $(PART_DEFAULT_MEMADDR) >> $@

$(ATF_PADDING_IMAGE_NAME): $(ATF_RAW_IMAGE_NAME) $(ATF_TEMP_PADDING_FILE) $(ATF_ADDITIONAL_DEPENDENCIES)
	@echo ATF build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) cat $(ATF_RAW_IMAGE_NAME) $(ATF_TEMP_PADDING_FILE) > $@

$(ATF_SIGNED_IMAGE_NAME): ALIGNMENT=512
$(ATF_SIGNED_IMAGE_NAME): $(ATF_PADDING_IMAGE_NAME) $(TRUSTZONE_SIGN_TOOL) $(TRUSTZONE_IMG_PROTECT_CFG) $(ATF_ADDITIONAL_DEPENDENCIES)
	@echo ATF build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(TRUSTZONE_SIGN_TOOL) $(TRUSTZONE_IMG_PROTECT_CFG) $(ATF_PADDING_IMAGE_NAME) $@ $(PART_DEFAULT_MEMADDR)
	$(hide) FILE_SIZE=$$(wc -c < "$(ATF_SIGNED_IMAGE_NAME)");REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
	if [ $${REMAINDER} -ne 0 ]; then echo "[ERROR] File $@ size $${FILE_SIZE} is not $(ALIGNMENT) bytes aligned";exit 1; fi

$(ATF_COMP_IMAGE_NAME): ALIGNMENT=512
$(ATF_COMP_IMAGE_NAME): $(ATF_SIGNED_IMAGE_NAME) $(MTK_MKIMAGE_TOOL) $(ATF_TEMP_CFG_FILE) $(ATF_ADDITIONAL_DEPENDENCIES)
	@echo ATF build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(MTK_MKIMAGE_TOOL) $(ATF_SIGNED_IMAGE_NAME) $(ATF_TEMP_CFG_FILE) > $@
	$(hide) FILE_SIZE=$$(wc -c < "$(ATF_COMP_IMAGE_NAME)");REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
	if [ $${REMAINDER} -ne 0 ]; then echo "[ERROR] File $@ size $${FILE_SIZE} is not $(ALIGNMENT) bytes aligned";exit 1; fi

