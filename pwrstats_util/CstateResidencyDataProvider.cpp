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
#define LOG_TAG "pwrstats_util"

#include "CstateResidencyDataProvider.h"

#include <regex>
#include <string>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <android-base/parsedouble.h>
#include <android-base/logging.h>

int CstateResidencyDataProvider::get(std::unordered_map<std::string, uint64_t>* data) {
    std::ifstream file("/sys/kernel/debug/lpm_stats/stats");

    std::smatch matches;
    const std::regex searchExpr("\\[(.*?)\\] (.*?):");
    std::string line;
    const std::string searchStr = "total success time:";

    while (std::getline(file, line)) {
        if (std::regex_search(line, matches, searchExpr)) {
            std::ostringstream key;
            key << matches[1] << "__" << matches[2];

            while (std::getline(file, line)) {
                size_t pos = line.find(searchStr);
                if (pos != std::string::npos) {
                    float val;
                    if (android::base::ParseFloat(line.substr(pos + searchStr.size()), &val)) {
                        data->emplace(key.str(), static_cast<uint64_t>(val * 1000));
                    } else {
                        LOG(ERROR) << __func__ << ": failed to parse c-state data";
                    }
                    break;
                }
            }
        }
    }

    return 0;
}