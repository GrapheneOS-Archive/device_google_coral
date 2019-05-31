LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_RRO_THEME := SettingsResOverlay_G020Q

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res

LOCAL_PACKAGE_NAME := SettingsResOverlay_G020Q

LOCAL_SDK_VERSION := system_current

LOCAL_PRODUCT_MODULE := true

include $(BUILD_RRO_PACKAGE)
