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

#  blob(s) necessary for flame hardware
PRODUCT_COPY_FILES := \
    vendor/qcom/flame/proprietary/com.qti.snapdragon.sdk.display.xml:system_ext/etc/permissions/com.qti.snapdragon.sdk.display.xml \
    vendor/qcom/flame/proprietary/com.qti.snapdragon.sdk.display.jar:system_ext/framework/com.qti.snapdragon.sdk.display.jar \
    vendor/qcom/flame/proprietary/com.qualcomm.qti.uceservice-V2.0-java.jar:system_ext/framework/com.qualcomm.qti.uceservice-V2.0-java.jar \
    vendor/qcom/flame/proprietary/vendor.qti.hardware.alarm-V1.0-java.jar:system_ext/framework/vendor.qti.hardware.alarm-V1.0-java.jar \
    vendor/qcom/flame/proprietary/vendor.qti.hardware.data.latency-V1.0-java.jar:system_ext/framework/vendor.qti.hardware.data.latency-V1.0-java.jar \
    vendor/qcom/flame/proprietary/vendor.qti.hardware.factory-V1.0-java.jar:system_ext/framework/vendor.qti.hardware.factory-V1.0-java.jar \
    vendor/qcom/flame/proprietary/vendor.qti.ims.callinfo-V1.0-java.jar:system_ext/framework/vendor.qti.ims.callinfo-V1.0-java.jar \
    vendor/qcom/flame/proprietary/vendor.qti.voiceprint-V1.0-java.jar:system_ext/framework/vendor.qti.voiceprint-V1.0-java.jar \
    vendor/qcom/flame/proprietary/lib64/libadsprpc_system.so:system_ext/lib64/libadsprpc_system.so \
    vendor/qcom/flame/proprietary/lib64/libcdsprpc_system.so:system_ext/lib64/libcdsprpc_system.so \
    vendor/qcom/flame/proprietary/lib64/libDiagService.so:system_ext/lib64/libDiagService.so \
    vendor/qcom/flame/proprietary/lib64/libdiag_system.so:system_ext/lib64/libdiag_system.so \
    vendor/qcom/flame/proprietary/lib64/libdisplayconfig.so:system_ext/lib64/libdisplayconfig.so \
    vendor/qcom/flame/proprietary/lib64/libmdsprpc_system.so:system_ext/lib64/libmdsprpc_system.so \
    vendor/qcom/flame/proprietary/lib64/libmmosal.so:system_ext/lib64/libmmosal.so \
    vendor/qcom/flame/proprietary/lib64/libOpenCL_system.so:system_ext/lib64/libOpenCL_system.so \
    vendor/qcom/flame/proprietary/lib64/libqmi_cci_system.so:system_ext/lib64/libqmi_cci_system.so \
    vendor/qcom/flame/proprietary/lib64/libsdsprpc_system.so:system_ext/lib64/libsdsprpc_system.so \
    vendor/qcom/flame/proprietary/libadsprpc_system.so:system_ext/lib/libadsprpc_system.so \
    vendor/qcom/flame/proprietary/libcdsprpc_system.so:system_ext/lib/libcdsprpc_system.so \
    vendor/qcom/flame/proprietary/libDiagService.so:system_ext/lib/libDiagService.so \
    vendor/qcom/flame/proprietary/libdiag_system.so:system_ext/lib/libdiag_system.so \
    vendor/qcom/flame/proprietary/libdisplayconfig.so:system_ext/lib/libdisplayconfig.so \
    vendor/qcom/flame/proprietary/libmdsprpc_system.so:system_ext/lib/libmdsprpc_system.so \
    vendor/qcom/flame/proprietary/libmmosal.so:system_ext/lib/libmmosal.so \
    vendor/qcom/flame/proprietary/libOpenCL_system.so:system_ext/lib/libOpenCL_system.so \
    vendor/qcom/flame/proprietary/libqmi_cci_system.so:system_ext/lib/libqmi_cci_system.so \
    vendor/qcom/flame/proprietary/libsdsprpc_system.so:system_ext/lib/libsdsprpc_system.so \

