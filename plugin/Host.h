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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unordered_map>
#include <memory>

#include <interfaces/IDeviceSettingsHost.h>

#include "hal/dHost.h"
#include "hal/dHostImpl.h"
#include "DeviceSettingsTypes.h"

class Host {
    using IPlatform = hal::dHost::IPlatform;
    using DefaultImpl = dHostImpl;

    std::shared_ptr<IPlatform> _platform;

public:
    Host(std::shared_ptr<IPlatform> platform = nullptr);

    static Host Create();

    Host(const Host&) = default;
    Host& operator=(const Host&) = default;
    Host(Host&&) = default;
    Host& operator=(Host&&) = default;

    uint32_t GetEDID(uint8_t edId[], const uint16_t edIdLength);
    uint32_t GetMS12ConfigType(string &ms12Config);

    IPlatform& platform() { return *_platform; }

private:
    void Platform_init();

public:
    void InitialiseHAL() { std::static_pointer_cast<DefaultImpl>(_platform)->InitialiseHAL(); }
};