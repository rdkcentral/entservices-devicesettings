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

#pragma once

#include "Module.h"

#include <memory>
#include <chrono>
#include <cstdint>

#include <com/com.h>
#include <core/core.h>
#include <plugins/plugins.h>

#include <interfaces/IDeviceSettingsHost.h>

#include "Host.h"

#include "DeviceSettingsTypes.h"

namespace WPEFramework {
namespace Plugin {

    class DeviceSettingsHostImpl {

    private:
        DeviceSettingsHostImpl(const DeviceSettingsHostImpl&) = delete;
        DeviceSettingsHostImpl& operator=(const DeviceSettingsHostImpl&) = delete;

    public:
        DeviceSettingsHostImpl();
        virtual ~DeviceSettingsHostImpl();

        static DeviceSettingsHostImpl* Create() {
            return new DeviceSettingsHostImpl();
        }

    public:
        Core::hresult GetEDID(uint8_t edId[], const uint16_t edIdLength);
        Core::hresult GetMS12ConfigType(string &ms12Config);

    private:
        mutable Core::CriticalSection _apiLock;

        Host _host;

    public:
        void InitialiseHAL() { _host.InitialiseHAL(); }
    };

} // namespace Plugin
} // namespace WPEFramework