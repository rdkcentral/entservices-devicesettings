/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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

#pragma once

#include <string>
#include <vector>

#include <interfaces/IDeviceSettingsAudio.h>
#include <interfaces/IDeviceSettingsFPD.h>
#include <interfaces/IDeviceSettingsVideoDevice.h>
#include <interfaces/IDeviceSettingsVideoPort.h>

#include "DeviceSettingsTypes.h"

namespace WPEFramework {
namespace Plugin {

// ============================================================================
// Common port-entry helpers
// ============================================================================

struct VideoPortEntry {
    std::string   name;
    std::string   typeName;
    VideoPortType type;
    int32_t       index;
};

struct AudioPortEntry {
    std::string   name;
    AudioPortType type;
    int32_t       index;
};

// ============================================================================
// VideoPortConfigStore
//   Populated by: LoadVideoPortConfig(Exchange::IDeviceSettingsVideoPort*, ...)
// ============================================================================

struct VideoPortConfigStore {
    std::vector<VideoPortTypeConfig> typeConfigs;
    std::vector<VideoPortPortConfig> portConfigs;
    std::vector<VideoPortResolution> resolutions;

    void Clear();
    bool IsEmpty() const;

    bool BuildVideoPortEntries(std::vector<VideoPortEntry>& entries) const;
    std::string GetDefaultVideoPortName() const;
    std::string GetDefaultResolution(const std::string& portName) const;
    bool GetConnectedAudioPort(const std::string& portName,
                               int32_t& connectedAudioType,
                               int32_t& connectedAudioIndex) const;
    bool GetTypeConfig(VideoPortType typeId, VideoPortTypeConfig& cfg) const;
    bool ResolveByName(const std::string& requestedPort,
                       VideoPortEntry& resolvedEntry) const;
    std::vector<VideoPortResolution> GetResolutions() const;
};

// ============================================================================
// AudioConfigStore
//   Populated by: LoadAudioConfig(Exchange::IDeviceSettingsAudio*, ...)
// ============================================================================

struct AudioConfigStore {
    std::vector<AudioTypeConfigInfo> typeConfigs;
    std::vector<AudioPortConfigInfo> portConfigs;

    void Clear();
    bool IsEmpty() const;

    bool BuildAudioPortEntries(std::vector<AudioPortEntry>& entries) const;
    std::string GetDefaultAudioPortName() const;
    bool GetTypeConfig(int32_t typeId, AudioTypeConfigInfo& cfg) const;
    bool IsHDMIOutPortPresent() const;
};

// ============================================================================
// VideoDeviceConfigStore
//   Populated by: LoadVideoDeviceConfig(Exchange::IDeviceSettingsVideoDevice*, ...)
// ============================================================================

struct VideoDeviceConfigStore {
    std::vector<VideoDeviceConfigInfo> deviceConfigs;

    void Clear();
    bool IsEmpty() const;

    std::vector<VideoDeviceConfigInfo> GetAllConfigs() const;
    bool GetConfig(int32_t index, VideoDeviceConfigInfo& cfg) const;
    size_t GetCount() const;
};

// ============================================================================
// FrontPanelConfigStore
//   Populated by: LoadFrontPanelConfig(Exchange::IDeviceSettingsFPD*, ...)
// ============================================================================

struct FrontPanelConfigStore {
    std::vector<FPDColorConfig>       colors;
    std::vector<FPDIndicatorConfig>   indicators;
    std::vector<FPDTextDisplayConfig> textDisplays;
    std::vector<FPDColorBinding>      colorBindings;

    void Clear();
    bool IsEmpty() const;

    std::vector<FPDIndicatorConfig>   GetIndicators()    const;
    std::vector<FPDColorConfig>       GetColors()        const;
    std::vector<FPDTextDisplayConfig> GetTextDisplays()  const;
    std::vector<FPDColorBinding>      GetColorBindings() const;
    bool GetIndicatorById(int32_t id, FPDIndicatorConfig& cfg) const;
    bool GetTextDisplayByName(const std::string& name, FPDTextDisplayConfig& cfg) const;
};

// ============================================================================
// Standalone load functions — one per component interface
// ============================================================================

bool LoadVideoPortConfig(Exchange::IDeviceSettingsVideoPort* iface,
                         VideoPortConfigStore& store);

bool LoadAudioConfig(Exchange::IDeviceSettingsAudio* iface,
                     AudioConfigStore& store);

bool LoadVideoDeviceConfig(Exchange::IDeviceSettingsVideoDevice* iface,
                           VideoDeviceConfigStore& store);

bool LoadFrontPanelConfig(Exchange::IDeviceSettingsFPD* iface,
                          FrontPanelConfigStore& store);

} // namespace Plugin
} // namespace WPEFramework
