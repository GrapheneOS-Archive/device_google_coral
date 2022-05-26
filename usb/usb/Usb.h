/*
 * Copyright (C) 2022 The Android Open Source Project
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

#pragma once

#include <android-base/file.h>
#include <aidl/android/hardware/usb/BnUsb.h>
#include <aidl/android/hardware/usb/BnUsbCallback.h>
#include <utils/Log.h>

#define UEVENT_MSG_LEN 2048
// The type-c stack waits for 4.5 - 5.5 secs before declaring a port non-pd.
// The -partner directory would not be created until this is done.
// Having a margin of ~3 secs for the directory and other related bookeeping
// structures created and uvent fired.
#define PORT_TYPE_TIMEOUT 8

namespace aidl {
namespace android {
namespace hardware {
namespace usb {

using ::aidl::android::hardware::usb::IUsbCallback;
using ::aidl::android::hardware::usb::PortRole;
using ::android::base::ReadFileToString;
using ::android::base::WriteStringToFile;
using ::android::sp;
using ::ndk::ScopedAStatus;
using ::std::shared_ptr;
using ::std::string;

constexpr char kGadgetName[] = "a600000.dwc3";
#define PULLUP_PATH "/config/usb_gadget/g1/UDC"
#define SOC_PATH "/sys/devices/platform/soc/a600000.ssusb/"
#define USB_DATA_PATH SOC_PATH "usb_data_enabled"

#define USB_POWER_LIMIT_PATH "/sys/devices/platform/soc/c440000.qcom,spmi/spmi-0/spmi0-02/c440000.qcom,spmi:qcom,pm8150b@2:qcom,usb-pdphy@1700/usbpd0/"
#define SINK_CURRENT_LIMIT_PATH USB_POWER_LIMIT_PATH "usb_limit_sink_current"
#define SINK_LIMIT_ENABLE_PATH USB_POWER_LIMIT_PATH "usb_limit_sink_enable"
#define SOURCE_LIMIT_ENABLE_PATH USB_POWER_LIMIT_PATH "usb_limit_source_enable"

struct Usb : public BnUsb {
    Usb();

    ScopedAStatus enableContaminantPresenceDetection(const std::string& in_portName,
            bool in_enable, int64_t in_transactionId) override;
    ScopedAStatus queryPortStatus(int64_t in_transactionId) override;
    ScopedAStatus setCallback(const shared_ptr<IUsbCallback>& in_callback) override;
    ScopedAStatus switchRole(const string& in_portName, const PortRole& in_role,
            int64_t in_transactionId) override;
    ScopedAStatus enableUsbData(const string& in_portName, bool in_enable,
            int64_t in_transactionId) override;
    ScopedAStatus enableUsbDataWhileDocked(const string& in_portName,
            int64_t in_transactionId) override;
    ScopedAStatus limitPowerTransfer(const string& in_portName, bool in_limit,
            int64_t in_transactionId) override;
    ScopedAStatus resetUsbPort(const string& in_portName, int64_t in_transactionId) override;

    std::shared_ptr<::aidl::android::hardware::usb::IUsbCallback> mCallback;
    // Protects mCallback variable
    pthread_mutex_t mLock;
    // Protects roleSwitch operation
    pthread_mutex_t mRoleSwitchLock;
    // Threads waiting for the partner to come back wait here
    pthread_cond_t mPartnerCV;
    // lock protecting mPartnerCV
    pthread_mutex_t mPartnerLock;
    // Variable to signal partner coming back online after type switch
    bool mPartnerUp;
    // Usb Data status
    bool mUsbDataEnabled;

  private:
    pthread_t mPoll;
};

} // namespace usb
} // namespace hardware
} // namespace android
} // aidl
