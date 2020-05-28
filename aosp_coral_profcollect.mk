#
# Copyright 2020 The Android Open Source Project
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

$(call inherit-product, device/google/coral/aosp_coral.mk)
PRODUCT_NAME := aosp_coral_profcollect

# Add profcollectd to PRODUCT_PACKAGES.
PRODUCT_PACKAGES += profcollectctl profcollectd

# Turn on PGO sampling cflags.
SAMPLING_PGO := true
