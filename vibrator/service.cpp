/*
 * Copyright (C) 2017 The Android Open Source Project
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
#define LOG_TAG "android.hardware.vibrator@1.3-service.coral"

#include <android/hardware/vibrator/1.3/IVibrator.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>

#include "Vibrator.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::vibrator::V1_3::IVibrator;
using android::hardware::vibrator::V1_3::implementation::Vibrator;
using namespace android;

static const uint32_t Q_FLOAT_TO_FIXED = 1 << 16;
static const float Q_INDEX_TO_FLOAT = 1.5f;
static const uint32_t Q_INDEX_TO_FIXED = Q_INDEX_TO_FLOAT * Q_FLOAT_TO_FIXED;
static const uint32_t Q_INDEX_OFFSET = 2.0f * Q_FLOAT_TO_FIXED;

static constexpr uint32_t Q_DEFAULT = 15.5 * Q_FLOAT_TO_FIXED;
static const std::vector<uint32_t> V_LEVELS_DEFAULT = {60, 70, 80, 90, 100, 76};

static constexpr char ACTIVATE_PATH[] = "/sys/class/leds/vibrator/activate";
static constexpr char DURATION_PATH[] = "/sys/class/leds/vibrator/duration";
static constexpr char STATE_PATH[] = "/sys/class/leds/vibrator/state";
static constexpr char EFFECT_INDEX_PATH[] = "/sys/class/leds/vibrator/device/cp_trigger_index";
static constexpr char EFFECT_QUEUE_PATH[] = "/sys/class/leds/vibrator/device/cp_trigger_queue";
static constexpr char DIGI_SCALE_PATH[] = "/sys/class/leds/vibrator/device/dig_scale";
static constexpr char ASP_ENABLE_PATH[] = "/sys/class/leds/vibrator/device/asp_enable";

// File path to the calibration file
static constexpr char CALIBRATION_FILEPATH[] = "/persist/haptics/cs40l25a.cal";

// Kernel ABIs for updating the calibration data
static constexpr char F0_CONFIG[] = "f0_measured";
static constexpr char REDC_CONFIG[] = "redc_measured";
static constexpr char Q_CONFIG[] = "q_measured";
static constexpr char Q_INDEX[] = "q_index";
static constexpr char VOLTAGES_CONFIG[] = "v_levels";
static constexpr char F0_FILEPATH[] = "/sys/class/leds/vibrator/device/f0_stored";
static constexpr char REDC_FILEPATH[] = "/sys/class/leds/vibrator/device/redc_stored";
static constexpr char Q_FILEPATH[] = "/sys/class/leds/vibrator/device/q_stored";

static std::string trim(const std::string &str, const std::string &whitespace = " \t") {
    const auto str_begin = str.find_first_not_of(whitespace);
    if (str_begin == std::string::npos) {
        return "";
    }

    const auto str_end = str.find_last_not_of(whitespace);
    const auto str_range = str_end - str_begin + 1;

    return str.substr(str_begin, str_range);
}

static void ltrim(std::istream &stream) {
    while (std::isspace(static_cast<char>(stream.get()))) {
    }
    stream.unget();
}

static bool loadCalibrationData(std::vector<uint32_t> &v_levels) {
    std::map<std::string, std::string> config_data;

    std::ofstream f0{F0_FILEPATH};
    if (!f0) {
        ALOGE("Failed to open %s (%d): %s", F0_FILEPATH, errno, strerror(errno));
    }

    std::ofstream redc{REDC_FILEPATH};
    if (!redc) {
        ALOGE("Failed to open %s (%d): %s", REDC_FILEPATH, errno, strerror(errno));
    }

    std::ofstream q{Q_FILEPATH};
    if (!q) {
        ALOGE("Failed to open %s (%d): %s", Q_FILEPATH, errno, strerror(errno));
    }

    std::ifstream cal_data{CALIBRATION_FILEPATH};
    if (!cal_data) {
        ALOGE("Failed to open %s (%d): %s", CALIBRATION_FILEPATH, errno, strerror(errno));
        return false;
    }

    for (std::string line; std::getline(cal_data, line);) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::istringstream is_line(line);
        std::string key, value;
        if (std::getline(is_line, key, ':')) {
            uint32_t i = 0;
            if (key == VOLTAGES_CONFIG) {
                ltrim(is_line);
                while (std::getline(is_line, value, ' ')) {
                    if (i >= v_levels.size()) {
                        ALOGE("invalid v_levels config!");
                        return false;
                    }
                    v_levels[i++] = std::stoul(value);
                }
                if (i != v_levels.size()) {
                    ALOGE("invalid v_levels config!");
                    return false;
                }
                continue;
            } else if (std::getline(is_line, value)) {
                config_data[trim(key)] = trim(value);
            }
        }
    }

    if (config_data.find(F0_CONFIG) != config_data.end()) {
        f0 << config_data[F0_CONFIG] << std::endl;
    }

    if (config_data.find(REDC_CONFIG) != config_data.end()) {
        redc << config_data[REDC_CONFIG] << std::endl;
    }

    if (config_data.find(Q_CONFIG) != config_data.end()) {
        q << config_data[Q_CONFIG] << std::endl;
    } else if (config_data.find(Q_INDEX) != config_data.end()) {
        q << std::stoul(config_data[Q_INDEX]) * Q_INDEX_TO_FIXED + Q_INDEX_OFFSET << std::endl;
    } else {
        q << Q_DEFAULT << std::endl;
    }

    return true;
}

status_t registerVibratorService() {
    // Calibration data
    std::vector<uint32_t> v_levels(V_LEVELS_DEFAULT);
    Vibrator::HwApi hwapi;

    // ostreams below are required
    hwapi.mActivate.open(ACTIVATE_PATH);
    if (!hwapi.mActivate) {
        ALOGE("Failed to open %s (%d): %s", ACTIVATE_PATH, errno, strerror(errno));
    }

    hwapi.mDuration.open(DURATION_PATH);
    if (!hwapi.mDuration) {
        ALOGE("Failed to open %s (%d): %s", DURATION_PATH, errno, strerror(errno));
    }

    hwapi.mState.open(STATE_PATH);
    if (!hwapi.mState) {
        ALOGE("Failed to open %s (%d): %s", STATE_PATH, errno, strerror(errno));
    }

    hwapi.mEffectIndex.open(EFFECT_INDEX_PATH);
    if (!hwapi.mEffectIndex) {
        ALOGE("Failed to open %s (%d): %s", EFFECT_INDEX_PATH, errno, strerror(errno));
    }

    hwapi.mEffectQueue.open(EFFECT_QUEUE_PATH);
    if (!hwapi.mEffectQueue) {
        ALOGE("Failed to open %s (%d): %s", EFFECT_QUEUE_PATH, errno, strerror(errno));
    }

    hwapi.mScale.open(DIGI_SCALE_PATH);
    if (!hwapi.mScale) {
        ALOGE("Failed to open %s (%d): %s", DIGI_SCALE_PATH, errno, strerror(errno));
    }

    hwapi.mAspEnable.open(ASP_ENABLE_PATH);
    if (!hwapi.mAspEnable) {
        ALOGE("Failed to open %s (%d): %s", ASP_ENABLE_PATH, errno, strerror(errno));
    }

    hwapi.mState << 1 << std::endl;
    if (!hwapi.mState) {
        ALOGE("Failed to set state (%d): %s", errno, strerror(errno));
    }

    if (!loadCalibrationData(v_levels)) {
        ALOGW("Failed to load calibration data");
    }

    sp<IVibrator> vibrator = new Vibrator(std::move(hwapi), std::move(v_levels));

    return vibrator->registerAsService();
}

int main() {
    configureRpcThreadpool(1, true);
    status_t status = registerVibratorService();

    if (status != OK) {
        return status;
    }

    joinRpcThreadpool();
}
