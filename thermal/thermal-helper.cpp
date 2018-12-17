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

#include <sstream>
#include <set>
#include <vector>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>

#include "thermal-helper.h"
#include <pixelthermal/ThermalConfigParser.h>

namespace android {
namespace hardware {
namespace thermal {
namespace V1_0 {
namespace implementation {

using android::base::StringPrintf;
using ::android::hardware::thermal::V1_0::CpuUsage;
using ::android::hardware::google::pixel::thermal::Sensors;
using ::android::hardware::google::pixel::thermal::SensorInfo;
using ::android::hardware::google::pixel::thermal::TemperatureType;
using ::android::hardware::google::pixel::thermal::ThrottlingThresholds;

constexpr char kThermalSensorsRoot[] = "/sys/devices/virtual/thermal";
constexpr char kCpuOnlineRoot[] = "/sys/devices/system/cpu";
constexpr char kCpuUsageFile[] = "/proc/stat";
constexpr char kCpuOnlineFileSuffix[] = "online";
constexpr char kThermalConfigPrefix[] = "/vendor/etc/thermal-engine-";
constexpr unsigned int kMaxCpus = 8;

// This is a golden set of thermal sensor type and their temperature types.
// Used when we read in sensor values.
const std::map<std::string, SensorInfo>
kValidThermalSensorInfoMap = {
    {"battery", {TemperatureType::BATTERY, true, NAN, 60.0, .001}},

    // CPU thermal sensors (see b/117461989 for details)
    {"cpu-0-0-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU0
    {"cpu-0-1-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU1
    {"cpu-0-2-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU2
    {"cpu-0-3-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU3
    {"cpu-1-0-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU4
    {"cpu-1-1-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU5
    {"cpu-1-2-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU6
    {"cpu-1-3-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU7
    {"cpu-1-4-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU4
    {"cpu-1-5-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU5
    {"cpu-1-6-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU6
    {"cpu-1-7-usr", {TemperatureType::CPU, true, 95.0, 125.0, .001}},  // CPU7

    // GPU thermal sensors
    {"gpuss-0-usr", {TemperatureType::GPU, true, 95.0, 125.0, .001}},
    {"gpuss-1-usr", {TemperatureType::GPU, true, 95.0, 125.0, .001}},

    // Skin sensors
    // NB: quiet-therm considered the main skin thermistor for the time being
    {"disp-therm",  {TemperatureType::SKIN, false, NAN, NAN, .001}},
    {"quiet-therm", {TemperatureType::SKIN, false, NAN, NAN, .001}},

    // Other sensors...
      // Cellular power amplifier
    {"pa-therm",    {TemperatureType::UNKNOWN, false, NAN, NAN, .001}},
      // USB-C
    {"usbc-therm",  {TemperatureType::UNKNOWN, false, NAN, NAN, .001}},
      // Crystal Oscillator
    {"xo-therm",    {TemperatureType::UNKNOWN, false, NAN, NAN, .001}},
};

namespace {

void parseCpuUsagesFileAndAssignUsages(hidl_vec<CpuUsage>* cpu_usages) {
    uint64_t cpu_num, user, nice, system, idle;
    std::string cpu_name;
    std::string data;
    if (!android::base::ReadFileToString(kCpuUsageFile, &data)) {
        LOG(ERROR) << "Error reading Cpu usage file: " << kCpuUsageFile;
        return;
    }

    std::istringstream stat_data(data);
    std::string line;
    while (std::getline(stat_data, line)) {
        if (line.find("cpu") == 0 && isdigit(line[3])) {
            // Split the string using spaces.
            std::vector<std::string> words = android::base::Split(line, " ");
            cpu_name = words[0];
            cpu_num = std::stoi(cpu_name.substr(3));

            if (cpu_num < kMaxCpus) {
                user = std::stoi(words[1]);
                nice = std::stoi(words[2]);
                system = std::stoi(words[3]);
                idle = std::stoi(words[4]);

                // Check if the CPU is online by reading the online file.
                std::string cpu_online_path = StringPrintf(
                    "%s/%s/%s", kCpuOnlineRoot, cpu_name.c_str(),
                    kCpuOnlineFileSuffix);
                std::string is_online;
                if (!android::base::ReadFileToString(
                    cpu_online_path, &is_online)) {
                    LOG(ERROR) << "Could not open Cpu online file: "
                               << cpu_online_path;
                    return;
                }
                is_online = android::base::Trim(is_online);

                (*cpu_usages)[cpu_num].name = cpu_name;
                (*cpu_usages)[cpu_num].active = user + nice + system;
                (*cpu_usages)[cpu_num].total = user + nice + system + idle;
                (*cpu_usages)[cpu_num].isOnline = (is_online == "1") ?
                    true : false;
            } else {
                LOG(ERROR) << "Unexpected cpu number: " << words[0];
                return;
            }
        }
    }
}

float getThresholdFromType(const TemperatureType type,
                           const ThrottlingThresholds& threshold) {
    switch (type) {
        case TemperatureType::CPU:
          return threshold.cpu;
        case TemperatureType::GPU:
          return threshold.gpu;
        case TemperatureType::BATTERY:
          return threshold.battery;
        case TemperatureType::SKIN:
          return threshold.ss;
        default:
          return NAN;
    }
}

}  // namespace

// This is a golden set of cooling device types and their corresponding sensor
// thernal zone name.
static const std::map<std::string, std::string>
kValidCoolingDeviceTypeMap = {             // see b/117461989 for details
    {"thermal-cpufreq-0", "cpu-0-0-usr"},  // CPU0 (Little core 0)
    {"thermal-cpufreq-1", "cpu-0-1-usr"},  // CPU1 (Little core 1)
    {"thermal-cpufreq-2", "cpu-0-2-usr"},  // CPU2 (Little core 2)
    {"thermal-cpufreq-3", "cpu-0-3-usr"},  // CPU3 (Little core 3)
    {"thermal-cpufreq-4", "cpu-1-0-usr"},  // CPU4 (Big core 0, thermistor 1/2)
    {"thermal-cpufreq-5", "cpu-1-1-usr"},  // CPU5 (Big core 1, thermistor 1/2)
    {"thermal-cpufreq-6", "cpu-1-2-usr"},  // CPU6 (Big core 2, thermistor 1/2)
    {"thermal-cpufreq-7", "cpu-1-3-usr"},  // CPU7 (Big core 3, thermistor 1/2)
};

void ThermalHelper::updateOverideThresholds() {
    for (const auto& sensorMap : kValidThermalSensorInfoMap) {
        if (sensorMap.second.is_override) {
            switch (sensorMap.second.type) {
                case TemperatureType::CPU:
                    thresholds_.cpu = sensorMap.second.throttling;
                    vr_thresholds_.cpu = sensorMap.second.throttling;
                    shutdown_thresholds_.cpu = sensorMap.second.shutdown;
                    break;
                case TemperatureType::GPU:
                    thresholds_.gpu = sensorMap.second.throttling;
                    vr_thresholds_.gpu = sensorMap.second.throttling;
                    shutdown_thresholds_.gpu = sensorMap.second.shutdown;
                    break;
                case TemperatureType::BATTERY:
                    thresholds_.battery = sensorMap.second.throttling;
                    vr_thresholds_.battery = sensorMap.second.throttling;
                    shutdown_thresholds_.battery = sensorMap.second.shutdown;
                    break;
                case TemperatureType::SKIN:
                    thresholds_.ss = sensorMap.second.throttling;
                    vr_thresholds_.ss = sensorMap.second.throttling;
                    shutdown_thresholds_.ss = sensorMap.second.shutdown;
                    break;
                default:
                    break;
            }
        }
    }
}

/*
 * Populate the sensor_name_to_file_map_ map by walking through the file tree,
 * reading the type file and assigning the temp file path to the map.  If we do
 * not succeed, abort.
 */
ThermalHelper::ThermalHelper() :
        is_initialized_(initializeSensorMap() && initializeCoolingDevices()) {
    if (!is_initialized_) {
        LOG(FATAL) << "ThermalHAL could not be initialized properly.";
    }

    std::string hw = android::base::GetProperty("ro.hardware", "");
    std::string rev = android::base::GetProperty("vendor.thermal.hw_mode", "");
    std::string thermal_config(kThermalConfigPrefix + hw + rev + ".conf");
    std::string vr_thermal_config(kThermalConfigPrefix + hw + "-vr" + rev + ".conf");
    InitializeThresholdsFromThermalConfig(thermal_config,
                                          vr_thermal_config,
                                          kValidThermalSensorInfoMap,
                                          &thresholds_,
                                          &shutdown_thresholds_,
                                          &vr_thresholds_);
    updateOverideThresholds();
}

std::vector<std::string> ThermalHelper::getCoolingDevicePaths() {
    std::vector<std::string> paths;
    for (const auto& entry : kValidCoolingDeviceTypeMap) {
        std::string path = cooling_devices_.getCoolingDevicePath(entry.first);
        if (!path.empty()) {
            paths.push_back(path + "/cur_state");
        }
    }
    return paths;
}

const std::map<std::string, std::string>&
ThermalHelper::getValidCoolingDeviceMap() const {
    return kValidCoolingDeviceTypeMap;
}

bool ThermalHelper::readCoolingDevice(
        const std::string& cooling_device, int* data) const {
    return cooling_devices_.getCoolingDeviceState(cooling_device, data);
}

bool ThermalHelper::readTemperature(
        const std::string& sensor_name, Temperature* out) const {
    // Read the file.  If the file can't be read temp will be empty string.
    std::string temp;
    std::string path;

    if (!thermal_sensors_.readSensorFile(sensor_name, &temp, &path)) {
        LOG(ERROR) << "readTemperature: sensor not found: " << sensor_name;
        return false;
    }

    if (temp.empty() && !path.empty()) {
        LOG(ERROR) << "readTemperature: failed to open file: " << path;
        return false;
    }

    SensorInfo sensor_info = kValidThermalSensorInfoMap.at(sensor_name);

    out->type = sensor_info.type;
    out->name = sensor_name;
    out->currentValue = std::stoi(temp) * sensor_info.multiplier;
    out->throttlingThreshold = getThresholdFromType(sensor_info.type, thresholds_);
    out->shutdownThreshold = getThresholdFromType(
        sensor_info.type, shutdown_thresholds_);
    out->vrThrottlingThreshold = getThresholdFromType(
        sensor_info.type, vr_thresholds_);

    LOG(DEBUG) << StringPrintf(
        "readTemperature: %d, %s, %g, %g, %g, %g",
        out->type, out->name.c_str(), out->currentValue,
        out->throttlingThreshold, out->shutdownThreshold,
        out->vrThrottlingThreshold);

    return true;
}

bool ThermalHelper::initializeSensorMap() {
    for (const auto& sensor_info : kValidThermalSensorInfoMap) {
        std::string sensor_name = sensor_info.first;
        std::string sensor_temp_path = StringPrintf(
            "%s/tz-by-name/%s/temp", kThermalSensorsRoot, sensor_name.c_str());
        if (!thermal_sensors_.addSensor(sensor_name, sensor_temp_path)) {
            LOG(ERROR) << "Could not add " << sensor_name << "to sensors map";
        }
    }
    if (kValidThermalSensorInfoMap.size() == thermal_sensors_.getNumSensors()) {
        return true;
    }
    return false;
}

bool ThermalHelper::initializeCoolingDevices() {
    for (const auto& cooling_device_info : kValidCoolingDeviceTypeMap) {
        std::string cooling_device_name = cooling_device_info.first;
        std::string cooling_device_path = StringPrintf(
            "%s/cdev-by-name/%s", kThermalSensorsRoot,
            cooling_device_name.c_str());

        if (!cooling_devices_.addCoolingDevice(
                cooling_device_name, cooling_device_path)) {
            LOG(ERROR) << "Could not add " << cooling_device_name
                       << "to cooling device map";
            continue;
        }

        int data;
        if (cooling_devices_.getCoolingDeviceState(
                cooling_device_name, &data)) {
            cooling_device_path_to_throttling_level_map_.emplace(
                cooling_devices_.getCoolingDevicePath(
                    cooling_device_name).append("/cur_state"),
                data);
        } else {
            LOG(ERROR) << "Could not read cooling device value.";
        }
    }

    if (kValidCoolingDeviceTypeMap.size() ==
            cooling_devices_.getNumCoolingDevices()) {
        return true;
    }
    return false;
}

bool ThermalHelper::fillTemperatures(hidl_vec<Temperature>* temperatures) {
    temperatures->resize(kValidThermalSensorInfoMap.size());
    int current_index = 0;
    for (const auto& name_type_pair : kValidThermalSensorInfoMap) {
        Temperature temp;
        if (readTemperature(name_type_pair.first, &temp)) {
            (*temperatures)[current_index] = temp;
        } else {
            LOG(ERROR) << "Error reading temperature for sensor: "
                       << name_type_pair.first;
            return false;
        }
        ++current_index;
    }
    return current_index > 0;
}

bool ThermalHelper::fillCpuUsages(hidl_vec<CpuUsage>* cpu_usages) {
    cpu_usages->resize(kMaxCpus);
    parseCpuUsagesFileAndAssignUsages(cpu_usages);
    return true;
}

std::string ThermalHelper::getSkinSensorType() {
    return "quiet-therm";
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace thermal
}  // namespace hardware
}  // namespace android
