/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ANDROID_HARDWARE_THERMAL_V1_0_THERMAL_H
#define ANDROID_HARDWARE_THERMAL_V1_0_THERMAL_H

#include <future>
#include <thread>

#include <android/hardware/thermal/1.0/IThermal.h>
#include <hidl/Status.h>
#include <hidl/MQDescriptor.h>

#include "thermal-helper.h"
#include <pixelthermal/device_file_watcher.h>

namespace android {
namespace hardware {
namespace thermal {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::thermal::V1_0::IThermal;
using ::android::hardware::Return;
using ::android::sp;
using ::android::wp;

// The thermalHAL implementation for B2C2. For more information on the high
// level design check out: go/pixel-2018-thermalhal-doc.
class Thermal : public IThermal, public hidl_death_recipient {
   public:
    Thermal();
    ~Thermal() = default;

    // Disallow copy and assign.
    Thermal(const Thermal&) = delete;
    void operator= (const Thermal&) = delete;

    // Methods from ::android::hardware::thermal::V1_0::IThermal.
    Return<void> getTemperatures(getTemperatures_cb _hidl_cb) override;
    Return<void> getCpuUsages(getCpuUsages_cb _hidl_cb) override;
    Return<void> getCoolingDevices(getCoolingDevices_cb _hidl_cb) override;

    // Methods from ::android::hardware::hidl_death_recipient.
    void serviceDied(uint64_t cookie, const wp<IBase>& who) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    Return<void> debug(
        const hidl_handle& fd, const hidl_vec<hidl_string>& args) override;

   private:
    ThermalHelper thermal_helper_;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace thermal
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_THERMAL_V1_0_THERMAL_H
