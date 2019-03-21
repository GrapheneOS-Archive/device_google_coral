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

typedef struct {
    uint32_t duration;
    const char *queue;
} RingtoneSpec;

static constexpr uint32_t WAVEFORM_SIMPLE_EFFECT_INDEX = 2;

static constexpr uint32_t WAVEFORM_TICK_EFFECT_LEVEL = 1;

static constexpr uint32_t WAVEFORM_CLICK_EFFECT_LEVEL = 2;

static constexpr uint32_t WAVEFORM_HEAVY_CLICK_EFFECT_LEVEL = 3;

static constexpr uint32_t WAVEFORM_DOUBLE_CLICK_SILENCE_MS = 100;

static constexpr uint32_t WAVEFORM_LONG_VIBRATION_EFFECT_LEVEL = 5;
static constexpr uint32_t WAVEFORM_LONG_VIBRATION_EFFECT_INDEX = 0;

static constexpr uint32_t WAVEFORM_TRIGGER_QUEUE_SCALE = 100;
static constexpr uint32_t WAVEFORM_TRIGGER_QUEUE_INDEX = 65534;

static constexpr uint8_t VOLTAGE_SCALE_MAX = 100;

static constexpr RingtoneSpec WAVEFORM_RINGTONES[] = {
    // The_big_adventure - RINGTONE_1
    {
        44268,
        "!!, 11.100, 392, 11.100, 392, 11.100, 260, 10!!,"
        "11.100, 1020, 1020, 688, 2!",
    },
    // Copycat - RINGTONE_2
    {
        38244,
        "!!, 11.100, 240, 11.100, 712, 11!!,"
        "940, 2!",
    },
    // Crackle - RINGTONE_3
    {
        30000,
        "212,"
        "!!, 11.100, 184, 11.100, 88, 11.100, 88, 11.100, 384, 3!!,"
        "1020, 536, 5!",
    },
    // Flutterby - RINGTONE_4
    {
        33376,
        "!!, 12.100, 296, 12.100, 1020, 260, 1!!,"
        "6!",
    },
    // Hotline - RINGTONE_5
    {
        33320,
        "!!, 12.100, 596, 12.100, 1020, 908, 1!!,"
        "4!",
    },
    // Leaps_and_bounds - RINGTONE_6
    {
        48000,
        "!!,11.100, 244, 11.100, 116, 11.100, 112,"
        "13.100, 276, 23!!, 1!",
    },
    // Lollipop - RINGTONE_7
    {
        30024,
        "!!, 11.100, 312, 11.100, 312, 11.100, 312, 11.100, 204,"
        "11.100, 88, 11!!,"
        "624, 1!",
    },
    // Lost_and_found - RINGTONE_8
    {
        58840,
        "!!, 12.100, 468, 12.100, 468,"
        "11.100, 408, 11.100, 408, 11.100, 408, 11.100, 408, 7!!,"
        "1020, 880, 1!",
    },
    // Mash_up - RINGTONE_9
    {
        31200,
        "!!, 11.100, 468, 11.100, 228, 11.100, 228, 11.100, 228,"
        "11.100, 228, 11.100, 472, 3!!,"
        "8, 3!",
    },
    // Monkey_around - RINGTONE_10
    {
        31560,
        "14.100, 20,"
        "!!, 15.100, 20, 7!!, !!, 15.80, 20, 7!!, !!, 15.60, 20, 7!!,"
        "892, 4!",
    },
    // Schools_out - RINGTONE_11
    {
        32088,
        "12.60, 506, 12.80, 596, 12.100, 596,"
        "1020, 564, 6!",
    },
    // Zen_too - RINGTONE_12
    {
        37464,
        "!!, 11.100, 388, 11.100, 92, 11.100, 316,"
        "12.100, 972, 7!!,"
        "972, 1!",
    },
};

static constexpr int8_t MAX_COLD_START_LATENCY_MS = 6; // I2C Transaction + DSP Return-From-Standby
static constexpr int8_t MAX_PAUSE_TIMING_ERROR_MS = 1; // ALERT Irq Handling

static constexpr float AMP_ATTENUATE_STEP_SIZE = 0.125f;
static constexpr float EFFECT_FREQUENCY_KHZ = 48.0f;

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
    return on(timeoutMs, WAVEFORM_LONG_VIBRATION_EFFECT_INDEX);
}

Return<Status> Vibrator::off() {
    mHwApi.activate << 0 << std::endl;
    if (!mHwApi.activate) {
        mHwApi.activate.clear();
        ALOGE("Failed to turn vibrator off (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<bool> Vibrator::supportsAmplitudeControl() {
    return (mHwApi.scale ? true : false);
}

Return<Status> Vibrator::setAmplitude(uint8_t amplitude) {
    uint8_t factor = mVolLevels[WAVEFORM_LONG_VIBRATION_EFFECT_LEVEL];
    uint8_t adjusted = std::round(amplitude * (factor / static_cast<float>(VOLTAGE_SCALE_MAX)));
    return setAmplitude(adjusted, UINT8_MAX);
}

Return<Status> Vibrator::setAmplitude(uint8_t amplitude, uint8_t maximum) {
    if (!amplitude) {
        return Status::BAD_VALUE;
    }

    int32_t scale = std::round((-20 * std::log10(amplitude / static_cast<float>(maximum))) /
                               (AMP_ATTENUATE_STEP_SIZE));

    mHwApi.scale << scale << std::endl;
    if (!mHwApi.scale) {
        mHwApi.scale.clear();
        ALOGE("Failed to set amplitude (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    return Status::OK;
}

// Methods from ::android::hardware::vibrator::V1_3::IVibrator follow.

Return<bool> Vibrator::supportsExternalControl() {
    return (mHwApi.aspEnable ? true : false);
}

Return<Status> Vibrator::setExternalControl(bool enabled) {
    mHwApi.aspEnable << enabled << std::endl;
    if (!mHwApi.aspEnable) {
        mHwApi.aspEnable.clear();
        ALOGE("Failed to set external control (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
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

Return<Status> Vibrator::getRingtoneDetails(Effect effect, EffectStrength strength,
                                            uint32_t *outTimeMs, uint32_t *outVolLevel,
                                            std::string *outEffectQueue) {
    uint32_t volLevel;
    const RingtoneSpec *ringtone;

    switch (effect) {
        case Effect::RINGTONE_1:
            // fall-through
        case Effect::RINGTONE_2:
            // fall-through
        case Effect::RINGTONE_3:
            // fall-through
        case Effect::RINGTONE_4:
            // fall-through
        case Effect::RINGTONE_5:
            // fall-through
        case Effect::RINGTONE_6:
            // fall-through
        case Effect::RINGTONE_7:
            // fall-through
        case Effect::RINGTONE_8:
            // fall-through
        case Effect::RINGTONE_9:
            // fall-through
        case Effect::RINGTONE_10:
            // fall-through
        case Effect::RINGTONE_11:
            // fall-through
        case Effect::RINGTONE_12:
            static_assert(
                ARRAY_SIZE(WAVEFORM_RINGTONES) >= static_cast<uint32_t>(Effect::RINGTONE_12) -
                                                      static_cast<uint32_t>(Effect::RINGTONE_1),
                "Ringtone array is too small!");
            ringtone = &WAVEFORM_RINGTONES[static_cast<uint32_t>(effect) -
                                           static_cast<uint32_t>(Effect::RINGTONE_1)];
            break;
        default:
            return Status::UNSUPPORTED_OPERATION;
    }

    switch (strength) {
        case EffectStrength::LIGHT:
            volLevel = VOLTAGE_SCALE_MAX / 3;
            break;
        case EffectStrength::MEDIUM:
            volLevel = VOLTAGE_SCALE_MAX / 2;
            break;
        case EffectStrength::STRONG:
            volLevel = VOLTAGE_SCALE_MAX;
            break;
        default:
            return Status::UNSUPPORTED_OPERATION;
    }

    *outTimeMs = ringtone->duration;
    *outVolLevel = volLevel;
    *outEffectQueue = ringtone->queue;

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
        case Effect::RINGTONE_1:
            // fall-through
        case Effect::RINGTONE_2:
            // fall-through
        case Effect::RINGTONE_3:
            // fall-through
        case Effect::RINGTONE_4:
            // fall-through
        case Effect::RINGTONE_5:
            // fall-through
        case Effect::RINGTONE_6:
            // fall-through
        case Effect::RINGTONE_7:
            // fall-through
        case Effect::RINGTONE_8:
            // fall-through
        case Effect::RINGTONE_9:
            // fall-through
        case Effect::RINGTONE_10:
            // fall-through
        case Effect::RINGTONE_11:
            // fall-through
        case Effect::RINGTONE_12:
            status = getRingtoneDetails(effect, strength, &timeMs, &volLevel, &effectQueue);
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

    setAmplitude(volLevel, VOLTAGE_SCALE_MAX);
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
