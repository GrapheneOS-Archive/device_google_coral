# Copyright 2019 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

PRODUCT_SOONG_NAMESPACES += \
    vendor/qcom/coral/proprietary \

# AOSP packages required by the blobs
PRODUCT_PACKAGES := \
    ims \
    QtiTelephonyService

PRODUCT_PACKAGES += \
    libadsprpc_system \
    libcdsprpc_system \
    libdiag_system \
    libdisplayconfig \
    libimsmedia_jni \
    libmdsprpc_system \
    libmmosal \
    libOpenCL_system \
    libqmi_cci_system \
    libsdsprpc_system \

#  blob(s) necessary for coral hardware
PRODUCT_COPY_FILES := \
    vendor/qcom/coral/proprietary/com.qti.snapdragon.sdk.display.jar:system_ext/framework/com.qti.snapdragon.sdk.display.jar \
    vendor/qcom/coral/proprietary/com.qti.snapdragon.sdk.display.xml:system_ext/etc/permissions/com.qti.snapdragon.sdk.display.xml \
    vendor/qcom/coral/proprietary/com.qualcomm.qti.uceservice-V2.0-java.jar:system_ext/framework/com.qualcomm.qti.uceservice-V2.0-java.jar \
    vendor/qcom/coral/proprietary/qcrilhook.jar:system_ext/framework/qcrilhook.jar \
    vendor/qcom/coral/proprietary/qcrilhook.xml:system_ext/etc/permissions/qcrilhook.xml \
    vendor/qcom/coral/proprietary/vendor.qti.hardware.alarm-V1.0-java.jar:system_ext/framework/vendor.qti.hardware.alarm-V1.0-java.jar \
    vendor/qcom/coral/proprietary/vendor.qti.hardware.data.latency-V1.0-java.jar:system_ext/framework/vendor.qti.hardware.data.latency-V1.0-java.jar \
    vendor/qcom/coral/proprietary/vendor.qti.hardware.factory-V1.0-java.jar:system_ext/framework/vendor.qti.hardware.factory-V1.0-java.jar \
    vendor/qcom/coral/proprietary/vendor.qti.ims.callinfo-V1.0-java.jar:system_ext/framework/vendor.qti.ims.callinfo-V1.0-java.jar \
    vendor/qcom/coral/proprietary/vendor.qti.voiceprint-V1.0-java.jar:system_ext/framework/vendor.qti.voiceprint-V1.0-java.jar \

