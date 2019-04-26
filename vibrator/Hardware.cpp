/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "Hardware.h"

#include <log/log.h>

#include <iostream>

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

template <typename T>
static void fileFromEnv(const char *env, T *stream) {
    auto file = std::getenv(env);

    stream->open(file);
    if (!*stream) {
        ALOGE("Failed to open %s:%s (%d): %s", env, file, errno, strerror(errno));
    }
}

static std::string trim(const std::string &str, const std::string &whitespace = " \t") {
    const auto str_begin = str.find_first_not_of(whitespace);
    if (str_begin == std::string::npos) {
        return "";
    }

    const auto str_end = str.find_last_not_of(whitespace);
    const auto str_range = str_end - str_begin + 1;

    return str.substr(str_begin, str_range);
}

HwApi::HwApi() {
    // ostreams below are required
    fileFromEnv("F0_FILEPATH", &mF0);
    fileFromEnv("REDC_FILEPATH", &mRedc);
    fileFromEnv("Q_FILEPATH", &mQ);
    fileFromEnv("ACTIVATE_PATH", &mActivate);
    fileFromEnv("DURATION_PATH", &mDuration);
    fileFromEnv("STATE_PATH", &mState);
    fileFromEnv("EFFECT_DURATION_PATH", &mEffectDuration);
    fileFromEnv("EFFECT_INDEX_PATH", &mEffectIndex);
    fileFromEnv("EFFECT_QUEUE_PATH", &mEffectQueue);
    fileFromEnv("EFFECT_SCALE_PATH", &mEffectScale);
    fileFromEnv("GLOBAL_SCALE_PATH", &mGlobalScale);
    fileFromEnv("ASP_ENABLE_PATH", &mAspEnable);
    fileFromEnv("GPIO_FALL_INDEX", &mGpioFallIndex);
    fileFromEnv("GPIO_FALL_SCALE", &mGpioFallScale);
    fileFromEnv("GPIO_RISE_INDEX", &mGpioRiseIndex);
    fileFromEnv("GPIO_RISE_SCALE", &mGpioRiseScale);
}

HwCal::HwCal() {
    std::ifstream calfile;

    fileFromEnv("CALIBRATION_FILEPATH", &calfile);

    for (std::string line; std::getline(calfile, line);) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::istringstream is_line(line);
        std::string key, value;
        if (std::getline(is_line, key, ':') && std::getline(is_line, value)) {
            mCalData[trim(key)] = trim(value);
        }
    }
}

}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
