/*
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2026 RDK Management
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

#include <gtest/gtest.h>

#include "DeviceSettingsImplementation.h"

using namespace WPEFramework;

namespace {

TEST(DeviceSettingsImpTest, ExposesMainInterface)
{
    Core::ProxyType<Plugin::DeviceSettingsImp> implementation = Core::ProxyType<Plugin::DeviceSettingsImp>::Create();

    Exchange::IDeviceSettings* deviceSettings = static_cast<Exchange::IDeviceSettings*>(
        implementation->QueryInterface(Exchange::IDeviceSettings::ID));
    ASSERT_NE(nullptr, deviceSettings);

    deviceSettings->Release();
}

TEST(DeviceSettingsImpTest, ExposesComponentInterfaces)
{
    Core::ProxyType<Plugin::DeviceSettingsImp> implementation = Core::ProxyType<Plugin::DeviceSettingsImp>::Create();

    Exchange::IDeviceSettingsFPD* fpd = static_cast<Exchange::IDeviceSettingsFPD*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsFPD::ID));
    Exchange::IDeviceSettingsHDMIIn* hdmiIn = static_cast<Exchange::IDeviceSettingsHDMIIn*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsHDMIIn::ID));
    Exchange::IDeviceSettingsAudio* audio = static_cast<Exchange::IDeviceSettingsAudio*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsAudio::ID));
    Exchange::IDeviceSettingsVideoPort* videoPort = static_cast<Exchange::IDeviceSettingsVideoPort*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsVideoPort::ID));
    Exchange::IDeviceSettingsVideoDevice* videoDevice = static_cast<Exchange::IDeviceSettingsVideoDevice*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsVideoDevice::ID));
    Exchange::IDeviceSettingsHost* host = static_cast<Exchange::IDeviceSettingsHost*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsHost::ID));
    Exchange::IDeviceSettingsCompositeIn* compositeIn = static_cast<Exchange::IDeviceSettingsCompositeIn*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsCompositeIn::ID));
    Exchange::IDeviceSettingsDisplay* display = static_cast<Exchange::IDeviceSettingsDisplay*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsDisplay::ID));

    EXPECT_NE(nullptr, fpd);
    EXPECT_NE(nullptr, hdmiIn);
    EXPECT_NE(nullptr, audio);
    EXPECT_NE(nullptr, videoPort);
    EXPECT_NE(nullptr, videoDevice);
    EXPECT_NE(nullptr, host);
    EXPECT_NE(nullptr, compositeIn);
    EXPECT_NE(nullptr, display);

    if (fpd != nullptr) {
        fpd->Release();
    }
    if (hdmiIn != nullptr) {
        hdmiIn->Release();
    }
    if (audio != nullptr) {
        audio->Release();
    }
    if (videoPort != nullptr) {
        videoPort->Release();
    }
    if (videoDevice != nullptr) {
        videoDevice->Release();
    }
    if (host != nullptr) {
        host->Release();
    }
    if (compositeIn != nullptr) {
        compositeIn->Release();
    }
    if (display != nullptr) {
        display->Release();
    }
}

} // namespace