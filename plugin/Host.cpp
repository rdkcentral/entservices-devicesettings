/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <functional>
#include <unistd.h>
#include <memory>

#include <core/IAction.h>
#include <core/Time.h>
#include <core/WorkerPool.h>

#include "secure_wrapper.h"
#include "Host.h"
#include "hal/dHostImpl.h"

Host::Host(std::shared_ptr<IPlatform> platform)
    : _platform(std::move(platform))
{
    Platform_init();
}

Host Host::Create() {
    return Host(std::make_shared<dHostImpl>());
}

void Host::Platform_init()
{
    DSLOG_INFO("Host Init - Setting up event callbacks");

    CallbackBundle bundle;
    if (_platform) {
        this->platform().setAllCallbacks(bundle);
        this->platform().getPersistenceValue();
    }
}

uint32_t Host::GetEDID(uint8_t edId[], const uint16_t edIdLength) {
    DSLOG_INFO("edIdLength=%u", edIdLength);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().GetEDID(edId, edIdLength);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - platform call completed successfully");
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
}

uint32_t Host::GetMS12ConfigType(string &ms12Config) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().GetMS12ConfigType(ms12Config);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - ms12Config='%s'", ms12Config.c_str());
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
} // namespace end