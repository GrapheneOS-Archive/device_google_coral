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

#define LOG_TAG "android.hardware.power.stats@1.0-service.pixel"

#include <android/log.h>
#include <hidl/HidlTransportSupport.h>

#include <pixelpowerstats/PowerStats.h>
#include <pixelpowerstats/GenericStateResidencyDataProvider.h>
#include <pixelpowerstats/WlanStateResidencyDataProvider.h>

#include "RailDataProvider.h"

using android::sp;
using android::status_t;
using android::OK;

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::power::stats::V1_0::IPowerStats;
using android::hardware::power::stats::V1_0::implementation::PowerStats;
using android::hardware::power::stats::V1_0::PowerEntityType;

// Pixel specific
using android::hardware::google::pixel::powerstats::RailDataProvider;
using android::hardware::google::pixel::powerstats::GenericStateResidencyDataProvider;
using android::hardware::google::pixel::powerstats::PowerEntityConfig;
using android::hardware::google::pixel::powerstats::StateResidencyConfig;
using android::hardware::google::pixel::powerstats::WlanStateResidencyDataProvider;
using android::hardware::google::pixel::powerstats::generateGenericStateResidencyConfigs;

int main(int /* argc */, char** /* argv */) {
    ALOGI("power.stats service 1.0 is starting.");


    PowerStats* service = new PowerStats();

    // Add rail data provider
    service->setRailDataProvider(std::make_unique<RailDataProvider>());

    // Add power entities related to rpmh
    const uint64_t RPM_CLK = 19200;  // RPM runs at 19.2Mhz. Divide by 19200 for msec
    std::function<uint64_t(uint64_t)> rpmConvertToMs = [](uint64_t a) { return a / RPM_CLK; };
    std::vector<StateResidencyConfig> rpmStateResidencyConfigs = {
        {.name = "Sleep",
         .entryCountSupported = true,
         .entryCountPrefix = "Sleep Count:",
         .totalTimeSupported = true,
         .totalTimePrefix = "Sleep Accumulated Duration:",
         .totalTimeTransform = rpmConvertToMs,
         .lastEntrySupported = true,
         .lastEntryPrefix = "Sleep Last Entered At:",
         .lastEntryTransform = rpmConvertToMs}};

    auto rpmSdp =
        std::make_shared<GenericStateResidencyDataProvider>("/sys/power/rpmh_stats/master_stats");

    uint32_t apssId = service->addPowerEntity("APSS", PowerEntityType::SUBSYSTEM);
    rpmSdp->addEntity(apssId, PowerEntityConfig("APSS", rpmStateResidencyConfigs));

    uint32_t mpssId = service->addPowerEntity("MPSS", PowerEntityType::SUBSYSTEM);
    rpmSdp->addEntity(mpssId, PowerEntityConfig("MPSS", rpmStateResidencyConfigs));

    uint32_t adspId = service->addPowerEntity("ADSP", PowerEntityType::SUBSYSTEM);
    rpmSdp->addEntity(adspId, PowerEntityConfig("ADSP", rpmStateResidencyConfigs));

    uint32_t cdspId = service->addPowerEntity("CDSP", PowerEntityType::SUBSYSTEM);
    rpmSdp->addEntity(cdspId, PowerEntityConfig("CDSP", rpmStateResidencyConfigs));

    uint32_t slpiId = service->addPowerEntity("SLPI", PowerEntityType::SUBSYSTEM);
    rpmSdp->addEntity(slpiId, PowerEntityConfig("SLPI", rpmStateResidencyConfigs));

    service->addStateResidencyDataProvider(std::move(rpmSdp));

    // Add SoC power entity
    StateResidencyConfig socStateConfig = {
        .entryCountSupported = true,
        .entryCountPrefix = "count:",
        .totalTimeSupported = true,
        .totalTimePrefix = "actual last sleep(msec):",
        .lastEntrySupported = false
    };
    std::vector<std::pair<std::string, std::string>> socStateHeaders = {
        std::make_pair("AOSD", "RPM Mode:aosd"),
        std::make_pair("CXSD", "RPM Mode:cxsd"),
        std::make_pair("DDR", "RPM Mode:ddr"),
    };

    auto socSdp =
        std::make_shared<GenericStateResidencyDataProvider>("/sys/power/system_sleep/stats");

    uint32_t socId = service->addPowerEntity("SoC", PowerEntityType::POWER_DOMAIN);
    socSdp->addEntity(socId,
        PowerEntityConfig(generateGenericStateResidencyConfigs(socStateConfig, socStateHeaders)));

    service->addStateResidencyDataProvider(std::move(socSdp));

    // Add WLAN power entity
    uint32_t wlanId = service->addPowerEntity("WLAN", PowerEntityType::SUBSYSTEM);
    auto wlanSdp =
        std::make_shared<WlanStateResidencyDataProvider>(wlanId, "/sys/kernel/wlan/power_stats");
    service->addStateResidencyDataProvider(std::move(wlanSdp));

    // Add Airbrush power entity
    StateResidencyConfig airStateConfig = {
        .entryCountSupported = true,
        .entryCountPrefix = "Cumulative count:",
        .totalTimeSupported = true,
        .totalTimePrefix = "Cumulative duration msec:",
        .lastEntrySupported = true,
        .lastEntryPrefix = "Last entry timestamp msec:",
    };
    std::vector<std::pair<std::string, std::string>> airStateHeaders = {
        std::make_pair("Active", "ACTIVE"),
        std::make_pair("Sleep", "SLEEP"),
        std::make_pair("Deep sleep", "DEEP SLEEP"),
        std::make_pair("Suspend", "SUSPEND"),
        std::make_pair("Off", "OFF"),
        std::make_pair("Unknown", "UNKNOWN"),
    };

    auto airSdp =
        std::make_shared<GenericStateResidencyDataProvider>(
            "/sys/devices/platform/soc/soc:abc-sm/state_stats");

    uint32_t airId = service->addPowerEntity("Airbrush", PowerEntityType::SUBSYSTEM);
    airSdp->addEntity(airId, PowerEntityConfig("Airbrush Subsystem Power Stats",
        generateGenericStateResidencyConfigs(airStateConfig, airStateHeaders)));

    service->addStateResidencyDataProvider(std::move(airSdp));

    // Configure the threadpool
    configureRpcThreadpool(1, true /*callerWillJoin*/);

    status_t status = service->registerAsService();
    if (status != OK) {
        ALOGE("Could not register service for power.stats HAL Iface (%d), exiting.", status);
        return 1;
    }

    ALOGI("power.stats service is ready");
    joinRpcThreadpool();

    // In normal operation, we don't expect the thread pool to exit
    ALOGE("power.stats service is shutting down");
    return 1;
}
