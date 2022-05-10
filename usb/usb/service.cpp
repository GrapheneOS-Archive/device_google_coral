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

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <utils/Log.h>

#include "Usb.h"

using ::aidl::android::hardware::usb::Usb;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<Usb> usb = ndk::SharedRefBase::make<Usb>();

    const std::string instance = std::string() + Usb::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(usb->asBinder().get(), instance.c_str());
    CHECK(status == STATUS_OK);

    ALOGV("AIDL USB HAL about to start");
    ABinderProcess_joinThreadPool();
    return -1; // Should never be reached
}
