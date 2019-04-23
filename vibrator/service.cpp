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

#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>

#include "Vibrator.h"
#include "utils.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::vibrator::V1_3::implementation::Vibrator;
using namespace android;

static const uint32_t Q_FLOAT_TO_FIXED = 1 << 16;
static const float Q_INDEX_TO_FLOAT = 1.5f;
static const uint32_t Q_INDEX_TO_FIXED = Q_INDEX_TO_FLOAT * Q_FLOAT_TO_FIXED;
static const uint32_t Q_INDEX_OFFSET = 2.0f * Q_FLOAT_TO_FIXED;

static constexpr uint32_t Q_DEFAULT = 15.5 * Q_FLOAT_TO_FIXED;
static const std::array<uint32_t, 6> V_LEVELS_DEFAULT = {60, 70, 80, 90, 100, 76};

static constexpr char F0_CONFIG[] = "f0_measured";
static constexpr char REDC_CONFIG[] = "redc_measured";
static constexpr char Q_CONFIG[] = "q_measured";
static constexpr char Q_INDEX[] = "q_index";
static constexpr char VOLTAGES_CONFIG[] = "v_levels";

template <typename T>
static void fileFromEnv(const char *env, T *stream) {
    auto file = std::getenv(env);

    stream->open(file);
    if (!*stream) {
        ALOGE("Failed to open %s:%s (%d): %s", env, file, errno, strerror(errno));
    }
}

class HwApi : public Vibrator::HwApi {
  public:
    HwApi();
    bool setF0(uint32_t value) override { return set(value, mF0); }
    bool setRedc(uint32_t value) override { return set(value, mRedc); }
    bool setQ(uint32_t value) override { return set(value, mQ); }
    bool setActivate(bool value) override { return set(value, mActivate); }
    bool setDuration(uint32_t value) override { return set(value, mDuration); }
    bool getEffectDuration(uint32_t *value) override { return get(value, mEffectDuration); }
    bool setEffectIndex(uint32_t value) override { return set(value, mEffectIndex); }
    bool setEffectQueue(std::string value) override { return set(value, mEffectQueue); }
    bool hasEffectScale() override { return has(mEffectScale); }
    bool setEffectScale(uint32_t value) override { return set(value, mEffectScale); }
    bool setGlobalScale(uint32_t value) override { return set(value, mGlobalScale); }
    bool setState(bool value) override { return set(value, mState); }
    bool hasAspEnable() override { return has(mAspEnable); }
    bool getAspEnable(bool *value) override { return get(value, mAspEnable); }
    bool setAspEnable(bool value) override { return set(value, mAspEnable); }
    bool setGpioFallIndex(uint32_t value) override { return set(value, mGpioFallIndex); }
    bool setGpioFallScale(uint32_t value) override { return set(value, mGpioFallScale); }
    bool setGpioRiseIndex(uint32_t value) override { return set(value, mGpioRiseIndex); }
    bool setGpioRiseScale(uint32_t value) override { return set(value, mGpioRiseScale); }

  private:
    bool has(std::ostream &stream) { return !!stream; }
    template <typename T>
    bool get(T *value, std::istream &stream) {
        bool ret;
        stream.seekg(0);
        stream >> *value;
        ret = !!stream;
        stream.clear();
        return ret;
    }
    template <typename T>
    bool set(const T &value, std::ostream &stream) {
        bool ret;
        stream << value << std::endl;
        if (!(ret = !!stream)) {
            stream.clear();
        }
        return ret;
    }

  private:
    std::ofstream mF0;
    std::ofstream mRedc;
    std::ofstream mQ;
    std::ofstream mActivate;
    std::ofstream mDuration;
    std::ifstream mEffectDuration;
    std::ofstream mEffectIndex;
    std::ofstream mEffectQueue;
    std::ofstream mEffectScale;
    std::ofstream mGlobalScale;
    std::ofstream mState;
    std::fstream mAspEnable;
    std::ofstream mGpioFallIndex;
    std::ofstream mGpioFallScale;
    std::ofstream mGpioRiseIndex;
    std::ofstream mGpioRiseScale;
};

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

static std::string trim(const std::string &str, const std::string &whitespace = " \t") {
    const auto str_begin = str.find_first_not_of(whitespace);
    if (str_begin == std::string::npos) {
        return "";
    }

    const auto str_end = str.find_last_not_of(whitespace);
    const auto str_range = str_end - str_begin + 1;

    return str.substr(str_begin, str_range);
}

class HwCal : public Vibrator::HwCal {
  public:
    HwCal();
    bool getF0(uint32_t *value) override { return get(F0_CONFIG, value); }
    bool getRedc(uint32_t *value) override { return get(REDC_CONFIG, value); }
    bool getQ(uint32_t *value) override {
        if (get(Q_CONFIG, value)) {
            return true;
        }
        if (get(Q_INDEX, value)) {
            *value = *value * Q_INDEX_TO_FIXED + Q_INDEX_OFFSET;
            return true;
        }
        *value = Q_DEFAULT;
        return true;
    }
    bool getVolLevels(std::array<uint32_t, 6> *value) override {
        if (get(VOLTAGES_CONFIG, value)) {
            return true;
        }
        *value = V_LEVELS_DEFAULT;
        return true;
    }

  private:
    template <typename T>
    static Enable_If_Iterable<T, true> unpack(std::istream &stream, T *value) {
        for (auto &entry : *value) {
            stream >> entry;
        }
    }

    template <typename T>
    static Enable_If_Iterable<T, false> unpack(std::istream &stream, T *value) {
        stream >> *value;
    }

    template <typename T>
    bool get(const char *key, T *value) {
        auto it = mCalData.find(key);
        if (it == mCalData.end()) {
            ALOGE("Missing %s config!", key);
            return false;
        }
        std::stringstream stream{it->second};
        unpack(stream, value);
        if (!stream || !stream.eof()) {
            ALOGE("Invalid %s config!", key);
            return false;
        }
        return true;
    }

  private:
    std::map<std::string, std::string> mCalData;
};

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

status_t registerVibratorService() {
    sp<Vibrator> vibrator = new Vibrator(std::make_unique<HwApi>(), std::make_unique<HwCal>());

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
