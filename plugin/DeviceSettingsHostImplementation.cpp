/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
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

#include "DeviceSettingsHostImplementation.h"

#include <syscall.h>
#include <chrono>

using namespace std;

namespace WPEFramework {
namespace Plugin {

    DeviceSettingsHostImpl::DeviceSettingsHostImpl() : 
        _apiLock(),
        _host(Host::Create())
    {
        LOGINFO("DeviceSettingsHostImpl Constructor - Instance Address: %p", this);
    }

    DeviceSettingsHostImpl::~DeviceSettingsHostImpl() {
        LOGINFO("DeviceSettingsHostImpl Destructor - Instance Address: %p", this);
    }

    Core::hresult DeviceSettingsHostImpl::GetEDID(uint8_t edId[], const uint16_t edIdLength)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _host.GetEDID(edId, edIdLength);
        if (result == Core::ERROR_NONE) {
            LOGINFO("GetEDID succeeded: edIdLength=%u", edIdLength);
        } else {
            LOGERR("GetEDID failed: edIdLength=%u, error=%u", edIdLength, result);
        }
        return result;
    }

    Core::hresult DeviceSettingsHostImpl::GetMS12ConfigType(string &ms12Config)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _host.GetMS12ConfigType(ms12Config);
        if (result == Core::ERROR_NONE) {
            LOGINFO("GetMS12ConfigType succeeded: ms12Config='%s'", ms12Config.c_str());
        } else {
            LOGERR("GetMS12ConfigType failed: error=%u", result);
        }
        return result;
    }

} // namespace Plugin
} // namespace WPEFramework