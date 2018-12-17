/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * *    * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __THERMAL_HELPER_H__
#define __THERMAL_HELPER_H__

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include <ftw.h>
#include <fnmatch.h>

#include <android/hardware/thermal/1.0/IThermal.h>
#include <pixelthermal/cooling_devices.h>
#include <pixelthermal/device_file_watcher.h>
#include <pixelthermal/sensors.h>
#include <pixelthermal/thermal_structs.h>

namespace android {
namespace hardware {
namespace thermal {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_vec;
using ::android::hardware::thermal::V1_0::CoolingDevice;
using ::android::hardware::thermal::V1_0::CpuUsage;
using ::android::hardware::thermal::V1_0::Temperature;
using ::android::hardware::thermal::V1_0::TemperatureType;
using ::android::hardware::google::pixel::thermal::CoolingDevices;
using ::android::hardware::google::pixel::thermal::DeviceFileWatcher;
using ::android::hardware::google::pixel::thermal::SensorInfo;
using ::android::hardware::google::pixel::thermal::Sensors;
using ::android::hardware::google::pixel::thermal::ThrottlingThresholds;

class ThermalHelper {
 public:
    ThermalHelper();
    ~ThermalHelper() = default;

    bool fillTemperatures(hidl_vec<Temperature>* temperatures);
    bool fillCpuUsages(hidl_vec<CpuUsage>* cpu_usages);

    // Dissallow copy and assign.
    ThermalHelper(const ThermalHelper&) = delete;
    void operator=(const ThermalHelper&) = delete;

    bool isInitializedOk() const { return is_initialized_; }

    // Returns a vector of all cooling devices that has been found on the
    // device.
    std::vector<std::string> getCoolingDevicePaths();

    // Read the temperature of a single sensor.
    bool readTemperature(
        const std::string& sensor_name, Temperature* out) const;

    // Read the value of a single cooling device.
    bool readCoolingDevice(const std::string& cooling_device, int* data) const;

    const std::map<std::string, std::string>& getValidCoolingDeviceMap() const;

 private:
    bool initializeSensorMap();
    bool initializeCoolingDevices();

    // We don't use a variable here because notifyIfThrottlingSeen() uses it and
    // we might end up calling it before having it initialized.
    static std::string getSkinSensorType();

    // Update setting not in thermal config
    void updateOverideThresholds();

    Sensors thermal_sensors_;
    CoolingDevices cooling_devices_;
    std::unordered_map<std::string, int>
        cooling_device_path_to_throttling_level_map_;
    ThrottlingThresholds thresholds_;
    ThrottlingThresholds vr_thresholds_;
    ThrottlingThresholds shutdown_thresholds_;
    const bool is_initialized_;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace thermal
}  // namespace hardware
}  // namespace android

#endif  // __THERMAL_HELPER_H__
