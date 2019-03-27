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

#define LOG_TAG "VibratorService"

#include <log/log.h>

#include <cutils/properties.h>
#include <hardware/hardware.h>
#include <hardware/vibrator.h>

#include "Vibrator.h"

#include <cinttypes>
#include <cmath>
#include <fstream>
#include <iostream>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

using Status = ::android::hardware::vibrator::V1_0::Status;
using EffectStrength = ::android::hardware::vibrator::V1_0::EffectStrength;

static constexpr uint32_t WAVEFORM_SIMPLE_EFFECT_INDEX = 2;

static constexpr uint32_t WAVEFORM_TICK_EFFECT_LEVEL = 1;

static constexpr uint32_t WAVEFORM_CLICK_EFFECT_LEVEL = 2;

static constexpr uint32_t WAVEFORM_HEAVY_CLICK_EFFECT_LEVEL = 3;

static constexpr uint32_t WAVEFORM_DOUBLE_CLICK_SILENCE_MS = 100;

static constexpr uint32_t WAVEFORM_LONG_VIBRATION_EFFECT_INDEX = 0;

static constexpr uint32_t WAVEFORM_TRIGGER_QUEUE_SCALE = 100;
static constexpr uint32_t WAVEFORM_TRIGGER_QUEUE_INDEX = 65534;

static constexpr uint32_t VOLTAGE_GLOBAL_SCALE_LEVEL = 5;
static constexpr uint8_t VOLTAGE_SCALE_MAX = 100;

static constexpr int8_t MAX_COLD_START_LATENCY_MS = 6; // I2C Transaction + DSP Return-From-Standby
static constexpr int8_t MAX_PAUSE_TIMING_ERROR_MS = 1; // ALERT Irq Handling

static constexpr float AMP_ATTENUATE_STEP_SIZE = 0.125f;
static constexpr float EFFECT_FREQUENCY_KHZ = 48.0f;

static uint8_t amplitudeToScale(uint8_t amplitude, uint8_t maximum) {
    return std::round((-20 * std::log10(amplitude / static_cast<float>(maximum))) /
                      (AMP_ATTENUATE_STEP_SIZE));
}

Vibrator::Vibrator(HwApi &&hwapi, std::vector<uint32_t> &&v_levels)
    : mHwApi(std::move(hwapi)), mVolLevels(std::move(v_levels)) {
    uint32_t effectDuration;

    mHwApi.effectIndex << WAVEFORM_SIMPLE_EFFECT_INDEX << std::endl;
    if (!mHwApi.effectIndex) {
        mHwApi.effectIndex.clear();
    }

    mHwApi.effectDuration.seekg(0);
    mHwApi.effectDuration >> effectDuration;
    mHwApi.effectDuration.clear();

    mSimpleEffectDuration = std::ceil(effectDuration / EFFECT_FREQUENCY_KHZ);

    const uint32_t scaleFall =
        amplitudeToScale(mVolLevels[WAVEFORM_CLICK_EFFECT_LEVEL], VOLTAGE_SCALE_MAX);
    const uint32_t scaleRise =
        amplitudeToScale(mVolLevels[WAVEFORM_HEAVY_CLICK_EFFECT_LEVEL], VOLTAGE_SCALE_MAX);

    mHwApi.gpioFallIndex << WAVEFORM_SIMPLE_EFFECT_INDEX << std::endl;
    mHwApi.gpioFallScale << scaleFall << std::endl;
    mHwApi.gpioRiseIndex << WAVEFORM_SIMPLE_EFFECT_INDEX << std::endl;
    mHwApi.gpioRiseScale << scaleRise << std::endl;
}

Return<Status> Vibrator::on(uint32_t timeoutMs, uint32_t effectIndex) {
    mHwApi.effectIndex << effectIndex << std::endl;
    if (!mHwApi.effectIndex) {
        mHwApi.effectIndex.clear();
    }
    mHwApi.duration << timeoutMs << std::endl;
    if (!mHwApi.duration) {
        mHwApi.duration.clear();
    }
    mHwApi.activate << 1 << std::endl;
    if (!mHwApi.activate) {
        mHwApi.activate.clear();
    }

    return Status::OK;
}

// Methods from ::android::hardware::vibrator::V1_1::IVibrator follow.
Return<Status> Vibrator::on(uint32_t timeoutMs) {
    if (MAX_COLD_START_LATENCY_MS <= UINT32_MAX - timeoutMs) {
        timeoutMs += MAX_COLD_START_LATENCY_MS;
    }
    setGlobalAmplitude(true);
    return on(timeoutMs, WAVEFORM_LONG_VIBRATION_EFFECT_INDEX);
}

Return<Status> Vibrator::off() {
    setGlobalAmplitude(false);
    mHwApi.activate << 0 << std::endl;
    if (!mHwApi.activate) {
        mHwApi.activate.clear();
        ALOGE("Failed to turn vibrator off (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<bool> Vibrator::supportsAmplitudeControl() {
    return !isUnderExternalControl() && mHwApi.effectScale;
}

Return<Status> Vibrator::setAmplitude(uint8_t amplitude) {
    if (!amplitude) {
        return Status::BAD_VALUE;
    }

    if (!isUnderExternalControl()) {
        return setEffectAmplitude(amplitude, UINT8_MAX);
    } else {
        return Status::UNSUPPORTED_OPERATION;
    }
}

Return<Status> Vibrator::setEffectAmplitude(uint8_t amplitude, uint8_t maximum) {
    int32_t scale = amplitudeToScale(amplitude, maximum);

    mHwApi.effectScale << scale << std::endl;
    if (!mHwApi.effectScale) {
        mHwApi.effectScale.clear();
        ALOGE("Failed to set effect amplitude (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    return Status::OK;
}

Return<Status> Vibrator::setGlobalAmplitude(bool set) {
    uint8_t amplitude = set ? mVolLevels[VOLTAGE_GLOBAL_SCALE_LEVEL] : VOLTAGE_SCALE_MAX;
    int32_t scale = amplitudeToScale(amplitude, VOLTAGE_SCALE_MAX);

    mHwApi.globalScale << scale << std::endl;
    if (!mHwApi.globalScale) {
        mHwApi.globalScale.clear();
        ALOGE("Failed to set global amplitude (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    return Status::OK;
}

// Methods from ::android::hardware::vibrator::V1_3::IVibrator follow.

Return<bool> Vibrator::supportsExternalControl() {
    return (mHwApi.aspEnable ? true : false);
}

Return<Status> Vibrator::setExternalControl(bool enabled) {
    setGlobalAmplitude(enabled);

    mHwApi.aspEnable << enabled << std::endl;
    if (!mHwApi.aspEnable) {
        mHwApi.aspEnable.clear();
        ALOGE("Failed to set external control (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

bool Vibrator::isUnderExternalControl() {
    bool isAspEnabled;
    mHwApi.aspEnable.seekg(0);
    mHwApi.aspEnable >> isAspEnabled;
    mHwApi.aspEnable.clear();
    return isAspEnabled;
}

Return<void> Vibrator::perform(V1_0::Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    return performEffect(static_cast<Effect>(effect), strength, _hidl_cb);
}

Return<void> Vibrator::perform_1_1(V1_1::Effect_1_1 effect, EffectStrength strength,
                                   perform_cb _hidl_cb) {
    return performEffect(static_cast<Effect>(effect), strength, _hidl_cb);
}

Return<void> Vibrator::perform_1_2(V1_2::Effect effect, EffectStrength strength,
                                   perform_cb _hidl_cb) {
    return performEffect(static_cast<Effect>(effect), strength, _hidl_cb);
}

Return<void> Vibrator::perform_1_3(Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    return performEffect(effect, strength, _hidl_cb);
}

Return<Status> Vibrator::getSimpleDetails(Effect effect, EffectStrength strength,
                                          uint32_t *outTimeMs, uint32_t *outVolLevel) {
    uint32_t timeMs;
    uint32_t volLevel;
    uint32_t volIndex;

    switch (effect) {
        case Effect::TEXTURE_TICK:
            // fall-through
        case Effect::TICK:
            volIndex = WAVEFORM_TICK_EFFECT_LEVEL;
            break;
        case Effect::CLICK:
            volIndex = WAVEFORM_CLICK_EFFECT_LEVEL;
            break;
        case Effect::HEAVY_CLICK:
            volIndex = WAVEFORM_HEAVY_CLICK_EFFECT_LEVEL;
            break;
        default:
            return Status::UNSUPPORTED_OPERATION;
    }

    switch (strength) {
        case EffectStrength::LIGHT:
            volLevel = mVolLevels[--volIndex];
            break;
        case EffectStrength::MEDIUM:
            volLevel = mVolLevels[volIndex];
            break;
        case EffectStrength::STRONG:
            volLevel = mVolLevels[++volIndex];
            break;
        default:
            return Status::UNSUPPORTED_OPERATION;
    }

    timeMs = mSimpleEffectDuration + MAX_COLD_START_LATENCY_MS;

    *outTimeMs = timeMs;
    *outVolLevel = volLevel;

    return Status::OK;
}

Return<Status> Vibrator::getCompoundDetails(Effect effect, EffectStrength strength,
                                            uint32_t *outTimeMs, uint32_t *outVolLevel,
                                            std::string *outEffectQueue) {
    Status status;
    uint32_t timeMs;
    std::ostringstream effectBuilder;
    uint32_t thisTimeMs;
    uint32_t thisVolLevel;

    switch (effect) {
        case Effect::DOUBLE_CLICK:
            timeMs = 0;

            status = getSimpleDetails(Effect::TICK, strength, &thisTimeMs, &thisVolLevel);
            if (status != Status::OK) {
                return status;
            }
            effectBuilder << WAVEFORM_SIMPLE_EFFECT_INDEX << "." << thisVolLevel;
            timeMs += thisTimeMs;

            effectBuilder << ",";

            effectBuilder << WAVEFORM_DOUBLE_CLICK_SILENCE_MS;
            timeMs += WAVEFORM_DOUBLE_CLICK_SILENCE_MS + MAX_PAUSE_TIMING_ERROR_MS;

            effectBuilder << ",";

            status = getSimpleDetails(Effect::CLICK, strength, &thisTimeMs, &thisVolLevel);
            if (status != Status::OK) {
                return status;
            }
            effectBuilder << WAVEFORM_SIMPLE_EFFECT_INDEX << "." << thisVolLevel;
            timeMs += thisTimeMs;

            break;
        default:
            return Status::UNSUPPORTED_OPERATION;
    }

    *outTimeMs = timeMs;
    *outVolLevel = WAVEFORM_TRIGGER_QUEUE_SCALE;
    *outEffectQueue = effectBuilder.str();

    return Status::OK;
}

Return<Status> Vibrator::setEffectQueue(const std::string &effectQueue) {
    mHwApi.effectQueue << effectQueue << std::endl;
    if (!mHwApi.effectQueue) {
        mHwApi.effectQueue.clear();
        ALOGE("Failed to write \"%s\" to effect queue (%d): %s", effectQueue.c_str(), errno,
              strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    return Status::OK;
}

Return<void> Vibrator::performEffect(Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    Status status = Status::OK;
    uint32_t timeMs = 0;
    uint32_t effectIndex;
    uint32_t volLevel;
    std::string effectQueue;

    switch (effect) {
        case Effect::TEXTURE_TICK:
            // fall-through
        case Effect::TICK:
            // fall-through
        case Effect::CLICK:
            // fall-through
        case Effect::HEAVY_CLICK:
            status = getSimpleDetails(effect, strength, &timeMs, &volLevel);
            break;
        case Effect::DOUBLE_CLICK:
            status = getCompoundDetails(effect, strength, &timeMs, &volLevel, &effectQueue);
            break;
        default:
            status = Status::UNSUPPORTED_OPERATION;
            break;
    }
    if (status != Status::OK) {
        goto exit;
    }

    if (!effectQueue.empty()) {
        status = setEffectQueue(effectQueue);
        if (status != Status::OK) {
            goto exit;
        }
        effectIndex = WAVEFORM_TRIGGER_QUEUE_INDEX;
    } else {
        effectIndex = WAVEFORM_SIMPLE_EFFECT_INDEX;
    }

    setEffectAmplitude(volLevel, VOLTAGE_SCALE_MAX);
    on(timeMs, effectIndex);

exit:

    _hidl_cb(status, timeMs);

    return Void();
}

}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
