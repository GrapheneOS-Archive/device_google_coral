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
    vendor/qcom/flame/proprietary \

# AOSP packages required by the blobs
PRODUCT_PACKAGES := \
    ims \
    qcrilmsgtunnel \
    QtiTelephonyService

PRODUCT_PACKAGES += \
    libdiag_system \
    libimsmedia_jni \
    libmmosal \

#  blob(s) necessary for flame hardware
PRODUCT_COPY_FILES := \
    vendor/qcom/flame/proprietary/com.qualcomm.qti.uceservice-V2.0-java.jar:system_ext/framework/com.qualcomm.qti.uceservice-V2.0-java.jar \
    vendor/qcom/flame/proprietary/qcrilhook.jar:system_ext/framework/qcrilhook.jar \
    vendor/qcom/flame/proprietary/qti-telephony-utils.jar:system_ext/framework/qti-telephony-utils.jar \
    vendor/qcom/flame/proprietary/qti-telephony-hidl-wrapper.jar:system_ext/framework/qti-telephony-hidl-wrapper.jar \
    vendor/qcom/flame/proprietary/qcrilhook.xml:system_ext/etc/permissions/qcrilhook.xml \
    vendor/qcom/flame/proprietary/qti_telephony_hidl_wrapper.xml:system_ext/etc/permissions/qti_telephony_hidl_wrapper.xml \
    vendor/qcom/flame/proprietary/com.qualcomm.qcrilmsgtunnel.xml:system_ext/etc/permissions/com.qualcomm.qcrilmsgtunnel.xml \
    vendor/qcom/flame/proprietary/org_codeaurora_ims.xml:system_ext/etc/permissions/org_codeaurora_ims.xml \
    vendor/qcom/flame/proprietary/qti_telephony_utils.xml:system_ext/etc/permissions/qti_telephony_utils.xml \
    vendor/qcom/flame/proprietary/vendor.qti.hardware.alarm-V1.0-java.jar:system_ext/framework/vendor.qti.hardware.alarm-V1.0-java.jar \
    vendor/qcom/flame/proprietary/vendor.qti.hardware.data.latency-V1.0-java.jar:system_ext/framework/vendor.qti.hardware.data.latency-V1.0-java.jar \
    vendor/qcom/flame/proprietary/vendor.qti.ims.callinfo-V1.0-java.jar:system_ext/framework/vendor.qti.ims.callinfo-V1.0-java.jar \
    vendor/qcom/flame/proprietary/vendor.qti.voiceprint-V1.0-java.jar:system_ext/framework/vendor.qti.voiceprint-V1.0-java.jar \

