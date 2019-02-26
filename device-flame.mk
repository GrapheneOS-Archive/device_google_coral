#
# Copyright 2016 The Android Open Source Project
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
#

PRODUCT_HARDWARE := flame

include device/google/coral/device-common.mk

DEVICE_PACKAGE_OVERLAYS += device/google/coral/flame/overlay

# Audio XMLs for flame
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_policy_volumes_flame.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml

PRODUCT_PRODUCT_PROPERTIES += ro.com.google.ime.height_ratio=1.2

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.surface_flinger.display_primary_red=0.571498,0.268889,0.000102 \
    ro.surface_flinger.display_primary_green=0.264549,0.688889,0.044777 \
    ro.surface_flinger.display_primary_blue=0.183287,0.073333,0.965525 \
    ro.surface_flinger.display_primary_white=0.950352,1.0,1.089366
