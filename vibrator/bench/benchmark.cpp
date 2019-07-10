/* * Copyright (C) 2019 The Android Open Source Project *
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

#include "benchmark/benchmark.h"

#include <android-base/file.h>

#include "Hardware.h"
#include "Vibrator.h"

using ::android::sp;
using ::android::hardware::hidl_enum_range;

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

using ::android::hardware::vibrator::V1_0::EffectStrength;
using ::android::hardware::vibrator::V1_0::Status;

class VibratorBench : public benchmark::Fixture {
  public:
    void SetUp(::benchmark::State & /*state*/) override {
        mFileMap["EFFECT_DURATION_PATH"] =
            std::filesystem::path(mFilesDir.path) / "EFFECT_DURATION_PATH";
        std::ofstream{mFileMap["EFFECT_DURATION_PATH"]} << ((uint32_t)std::rand() ?: 1)
                                                        << std::endl;

        mFileMap["ASP_ENABLE_PATH"] = std::filesystem::path(mFilesDir.path) / "ASP_ENABLE_PATH";
        std::ofstream{mFileMap["ASP_ENABLE_PATH"]} << 0 << std::endl;

        setenv("CALIBRATION_FILEPATH", "/dev/null", true);
        setenv("F0_FILEPATH", "/dev/null", true);
        setenv("REDC_FILEPATH", "/dev/null", true);
        setenv("Q_FILEPATH", "/dev/null", true);
        setenv("ACTIVATE_PATH", "/dev/null", true);
        setenv("DURATION_PATH", "/dev/null", true);
        setenv("STATE_PATH", "/dev/null", true);
        setenv("EFFECT_INDEX_PATH", "/dev/null", true);
        setenv("EFFECT_QUEUE_PATH", "/dev/null", true);
        setenv("EFFECT_SCALE_PATH", "/dev/null", true);
        setenv("GLOBAL_SCALE_PATH", "/dev/null", true);
        setenv("GPIO_FALL_INDEX", "/dev/null", true);
        setenv("GPIO_FALL_SCALE", "/dev/null", true);
        setenv("GPIO_RISE_INDEX", "/dev/null", true);
        setenv("GPIO_RISE_SCALE", "/dev/null", true);

        setenv("EFFECT_DURATION_PATH", mFileMap["EFFECT_DURATION_PATH"].c_str(), true);
        setenv("ASP_ENABLE_PATH", mFileMap["ASP_ENABLE_PATH"].c_str(), true);

        mVibrator = new Vibrator(std::make_unique<HwApi>(), std::make_unique<HwCal>());
    }

    static void DefaultArgs(benchmark::internal::Benchmark *b) {
        b->Unit(benchmark::kMicrosecond);
    }

    static void SupportedEffectArgs(benchmark::internal::Benchmark *b) {
        b->ArgNames({"Effect", "Strength"});
        for (const auto &effect : hidl_enum_range<Effect>()) {
            for (const auto &strength : hidl_enum_range<EffectStrength>()) {
                b->Args({static_cast<long>(effect), static_cast<long>(strength)});
            }
        }
    }

  protected:
    TemporaryDir mFilesDir;
    std::map<std::string, std::string> mFileMap;
    sp<IVibrator> mVibrator;
};

#define BENCHMARK_WRAPPER(fixt, test, code) \
    BENCHMARK_DEFINE_F(fixt, test)          \
    /* NOLINTNEXTLINE */                    \
    (benchmark::State & state){code} BENCHMARK_REGISTER_F(fixt, test)->Apply(fixt::DefaultArgs)

BENCHMARK_WRAPPER(VibratorBench, on, {
    uint32_t duration = std::rand() ?: 1;

    for (auto _ : state) {
        mVibrator->on(duration);
    }
});

BENCHMARK_WRAPPER(VibratorBench, off, {
    for (auto _ : state) {
        mVibrator->off();
    }
});

BENCHMARK_WRAPPER(VibratorBench, supportsAmplitudeControl, {
    for (auto _ : state) {
        mVibrator->supportsAmplitudeControl();
    }
});

BENCHMARK_WRAPPER(VibratorBench, setAmplitude, {
    uint8_t amplitude = std::rand() ?: 1;

    for (auto _ : state) {
        mVibrator->setAmplitude(amplitude);
    }
});

BENCHMARK_WRAPPER(VibratorBench, supportsExternalControl, {
    for (auto _ : state) {
        mVibrator->supportsExternalControl();
    }
});

BENCHMARK_WRAPPER(VibratorBench, setExternalControl_enable, {
    for (auto _ : state) {
        mVibrator->setExternalControl(true);
    }
});

BENCHMARK_WRAPPER(VibratorBench, setExternalControl_disable, {
    for (auto _ : state) {
        mVibrator->setExternalControl(false);
    }
});

BENCHMARK_WRAPPER(VibratorBench, supportsExternalAmplitudeControl, {
    for (auto _ : state) {
        mVibrator->supportsExternalControl();
    }
});

BENCHMARK_WRAPPER(VibratorBench, perform_1_3,
                  {
                      Effect effect = Effect(state.range(0));
                      EffectStrength strength = EffectStrength(state.range(1));
                      bool supported = true;

                      mVibrator->perform_1_3(effect, strength,
                                             [&](Status status, uint32_t /*lengthMs*/) {
                                                 if (status == Status::UNSUPPORTED_OPERATION) {
                                                     supported = false;
                                                 }
                                             });

                      if (!supported) {
                          return;
                      }

                      for (auto _ : state) {
                          mVibrator->perform_1_3(effect, strength,
                                                 [](Status /*status*/, uint32_t /*lengthMs*/) {});
                      }
                  })
    ->Apply(VibratorBench::SupportedEffectArgs);

}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android

BENCHMARK_MAIN();
