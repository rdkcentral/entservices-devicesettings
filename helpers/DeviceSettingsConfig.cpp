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

#include "DeviceSettingsConfig.h"

#include <algorithm>
#include <cctype>

#include "DeviceSettingsImplementation.h"
#include "UtilsLogging.h"

namespace WPEFramework {
namespace Plugin {

// ============================================================================
// Public: Refresh (calls all four individual refresh methods)
// ============================================================================

bool DeviceSettingsConfig::Refresh(DeviceSettingsImp* deviceSettings)
{
    if (deviceSettings == nullptr) {
        LOGERR("DeviceSettingsConfig::Refresh: DeviceSettings implementation not available");
        return false;
    }

    bool ok = true;
    ok &= RefreshVideoPortConfig(deviceSettings);
    ok &= RefreshAudioConfig(deviceSettings);
    ok &= RefreshVideoDeviceConfig(deviceSettings);
    ok &= RefreshFrontPanelConfig(deviceSettings);
    return ok;
}

bool DeviceSettingsConfig::IsCacheEmpty() const
{
    _lock.Lock();
    const bool empty = _cachedVideoPortConfigs.empty()
                    && _cachedAudioPortConfigs.empty()
                    && _cachedVideoDeviceConfigs.empty()
                    && _cachedFPDIndicators.empty();
    _lock.Unlock();
    return empty;
}

// ============================================================================
// Private: four individual refresh methods
// ============================================================================

bool DeviceSettingsConfig::RefreshVideoPortConfig(DeviceSettingsImp* deviceSettings)
{
    std::vector<VideoPortTypeConfig>  videoPortTypes;
    std::vector<VideoPortPortConfig>  videoPorts;
    std::vector<VideoPortResolution>  videoResolutions;

    IVideoPortTypeConfigIterator* typeIt = nullptr;
    IVideoPortPortConfigIterator* portIt = nullptr;
    IVideoPortResolutionIterator* resIt  = nullptr;

    const uint32_t result = deviceSettings->GetVideoPortConfig(typeIt, portIt, resIt);
    if (result != Core::ERROR_NONE) {
        LOGERR("DeviceSettingsConfig::RefreshVideoPortConfig: GetVideoPortConfig failed: %u", result);
        return false;
    }

    if (typeIt != nullptr) {
        VideoPortTypeConfig cfg;
        while (typeIt->Next(cfg)) {
            videoPortTypes.push_back(cfg);
        }
        typeIt->Release();
    }

    if (portIt != nullptr) {
        VideoPortPortConfig cfg;
        while (portIt->Next(cfg)) {
            videoPorts.push_back(cfg);
        }
        portIt->Release();
    }

    if (resIt != nullptr) {
        VideoPortResolution res;
        while (resIt->Next(res)) {
            videoResolutions.push_back(res);
        }
        resIt->Release();
    }

    _lock.Lock();
    _cachedVideoPortTypes.swap(videoPortTypes);
    _cachedVideoPortConfigs.swap(videoPorts);
    _cachedVideoPortResolutions.swap(videoResolutions);
    _lock.Unlock();

    LOGINFO("DeviceSettingsConfig::RefreshVideoPortConfig: types=%zu ports=%zu resolutions=%zu",
            _cachedVideoPortTypes.size(), _cachedVideoPortConfigs.size(),
            _cachedVideoPortResolutions.size());
    return true;
}

bool DeviceSettingsConfig::RefreshAudioConfig(DeviceSettingsImp* deviceSettings)
{
    std::vector<AudioTypeConfigInfo> audioTypes;
    std::vector<AudioPortConfigInfo> audioPorts;

    IAudioTypeConfigIterator* typeIt = nullptr;
    IAudioPortConfigIterator* portIt = nullptr;

    const uint32_t result = deviceSettings->GetAudioConfig(typeIt, portIt);
    if (result != Core::ERROR_NONE) {
        LOGERR("DeviceSettingsConfig::RefreshAudioConfig: GetAudioConfig failed: %u", result);
        return false;
    }

    if (typeIt != nullptr) {
        AudioTypeConfigInfo cfg;
        while (typeIt->Next(cfg)) {
            audioTypes.push_back(cfg);
        }
        typeIt->Release();
    }

    if (portIt != nullptr) {
        AudioPortConfigInfo cfg;
        while (portIt->Next(cfg)) {
            audioPorts.push_back(cfg);
        }
        portIt->Release();
    }

    _lock.Lock();
    _cachedAudioTypeConfigs.swap(audioTypes);
    _cachedAudioPortConfigs.swap(audioPorts);
    _lock.Unlock();

    LOGINFO("DeviceSettingsConfig::RefreshAudioConfig: audioTypes=%zu audioPorts=%zu",
            _cachedAudioTypeConfigs.size(), _cachedAudioPortConfigs.size());
    return true;
}

bool DeviceSettingsConfig::RefreshVideoDeviceConfig(DeviceSettingsImp* deviceSettings)
{
    std::vector<VideoDeviceConfigInfo> videoDevices;

    IVideoDeviceConfigIterator* it = nullptr;

    const uint32_t result = deviceSettings->GetVideoDeviceConfig(it);
    if (result != Core::ERROR_NONE) {
        LOGERR("DeviceSettingsConfig::RefreshVideoDeviceConfig: GetVideoDeviceConfig failed: %u", result);
        return false;
    }

    if (it != nullptr) {
        VideoDeviceConfigInfo cfg;
        while (it->Next(cfg)) {
            videoDevices.push_back(cfg);
        }
        it->Release();
    }

    _lock.Lock();
    _cachedVideoDeviceConfigs.swap(videoDevices);
    _lock.Unlock();

    LOGINFO("DeviceSettingsConfig::RefreshVideoDeviceConfig: devices=%zu",
            _cachedVideoDeviceConfigs.size());
    return true;
}

bool DeviceSettingsConfig::RefreshFrontPanelConfig(DeviceSettingsImp* deviceSettings)
{
    std::vector<FPDTextDisplayConfig> textDisplays;
    std::vector<FPDIndicatorConfig>   indicators;
    std::vector<FPDColorConfig>       colors;
    std::vector<FPDColorBinding>      colorBindings;

    IFPDTextDisplayConfigIterator* textIt    = nullptr;
    IFPDIndicatorConfigIterator*   indicIt   = nullptr;
    IFPDColorConfigIterator*       colorIt   = nullptr;
    IFPDColorBindingIterator*      bindingIt = nullptr;

    const uint32_t result = deviceSettings->GetFrontPanelConfig(textIt, indicIt, colorIt, bindingIt);
    if (result != Core::ERROR_NONE) {
        LOGERR("DeviceSettingsConfig::RefreshFrontPanelConfig: GetFrontPanelConfig failed: %u", result);
        return false;
    }

    if (textIt != nullptr) {
        FPDTextDisplayConfig cfg;
        while (textIt->Next(cfg)) {
            textDisplays.push_back(cfg);
        }
        textIt->Release();
    }

    if (indicIt != nullptr) {
        FPDIndicatorConfig cfg;
        while (indicIt->Next(cfg)) {
            indicators.push_back(cfg);
        }
        indicIt->Release();
    }

    if (colorIt != nullptr) {
        FPDColorConfig cfg;
        while (colorIt->Next(cfg)) {
            colors.push_back(cfg);
        }
        colorIt->Release();
    }

    if (bindingIt != nullptr) {
        FPDColorBinding cfg;
        while (bindingIt->Next(cfg)) {
            colorBindings.push_back(cfg);
        }
        bindingIt->Release();
    }

    _lock.Lock();
    _cachedFPDTextDisplays.swap(textDisplays);
    _cachedFPDIndicators.swap(indicators);
    _cachedFPDColors.swap(colors);
    _cachedFPDColorBindings.swap(colorBindings);
    _lock.Unlock();

    LOGINFO("DeviceSettingsConfig::RefreshFrontPanelConfig: textDisplays=%zu indicators=%zu colors=%zu bindings=%zu",
            _cachedFPDTextDisplays.size(), _cachedFPDIndicators.size(),
            _cachedFPDColors.size(), _cachedFPDColorBindings.size());
    return true;
}

// ============================================================================
// VideoPort queries
// ============================================================================

bool DeviceSettingsConfig::BuildVideoPortEntries(std::vector<VideoPortEntry>& entries) const
{
    entries.clear();

    std::vector<VideoPortTypeConfig>  videoPortTypes;
    std::vector<VideoPortPortConfig>  videoPortConfigs;

    _lock.Lock();
    videoPortTypes   = _cachedVideoPortTypes;
    videoPortConfigs = _cachedVideoPortConfigs;
    _lock.Unlock();

    for (size_t i = 0; i < videoPortConfigs.size(); ++i) {
        const VideoPortPortConfig& portConfig = videoPortConfigs[i];

        // Find matching type config to get the type name
        std::string typeName;
        for (size_t j = 0; j < videoPortTypes.size(); ++j) {
            if (videoPortTypes[j].typeId == portConfig.videoPortType) {
                typeName = videoPortTypes[j].name;
                break;
            }
        }

        VideoPortEntry entry;
        entry.type     = portConfig.videoPortType;
        entry.index    = portConfig.videoPortIndex;
        entry.typeName = typeName;
        entry.name     = BuildVideoPortName(typeName, portConfig.videoPortIndex);
        entries.push_back(entry);
    }

    return !entries.empty();
}

std::string DeviceSettingsConfig::GetDefaultVideoPortName() const
{
    // Mirrors device::Host::getDefaultVideoPortName():
    //   Preference order: HDMI (index 0) > INTERNAL (index 0) > first port.
    std::vector<VideoPortEntry> entries;
    if (!BuildVideoPortEntries(entries)) {
        return std::string("HDMI0");
    }

    std::string defaultName = entries[0].name; // fallback: first port
    bool found = false;

    for (size_t i = 0; i < entries.size() && !found; ++i) {
        if (entries[i].type == VideoPortType::DS_VIDEO_PORT_TYPE_HDMI && entries[i].index == 0) {
            defaultName = entries[i].name;
            found = true;
        }
    }

    for (size_t i = 0; i < entries.size() && !found; ++i) {
        if (entries[i].type == VideoPortType::DS_VIDEO_PORT_TYPE_INTERNAL && entries[i].index == 0) {
            defaultName = entries[i].name;
            found = true;
        }
    }

    return defaultName;
}

bool DeviceSettingsConfig::IsHDMIOutPortPresent() const
{
    // Mirrors device::Host::isHDMIOutPortPresent():
    // True if any audio port with name containing "HDMI0" exists.
    std::vector<AudioPortEntry> audioEntries;
    if (!BuildAudioPortEntries(audioEntries)) {
        return false;
    }
    for (size_t i = 0; i < audioEntries.size(); ++i) {
        if (audioEntries[i].name.find("HDMI0") != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::string DeviceSettingsConfig::GetVideoPortDefaultResolution(const std::string& portName) const
{
    std::vector<VideoPortTypeConfig>  videoPortTypes;
    std::vector<VideoPortPortConfig>  videoPortConfigs;

    _lock.Lock();
    videoPortTypes   = _cachedVideoPortTypes;
    videoPortConfigs = _cachedVideoPortConfigs;
    _lock.Unlock();

    for (size_t i = 0; i < videoPortConfigs.size(); ++i) {
        const VideoPortPortConfig& pc = videoPortConfigs[i];

        // Construct name to compare
        std::string typeName;
        for (size_t j = 0; j < videoPortTypes.size(); ++j) {
            if (videoPortTypes[j].typeId == pc.videoPortType) {
                typeName = videoPortTypes[j].name;
                break;
            }
        }
        const std::string name = BuildVideoPortName(typeName, pc.videoPortIndex);
        if (EqualsIgnoreCase(name, portName)) {
            return pc.defaultResolution;
        }
    }
    return std::string();
}

bool DeviceSettingsConfig::GetVideoPortConnectedAudioPort(const std::string& portName,
                                                      int32_t& connectedAudioType,
                                                      int32_t& connectedAudioIndex) const
{
    std::vector<VideoPortTypeConfig>  videoPortTypes;
    std::vector<VideoPortPortConfig>  videoPortConfigs;

    _lock.Lock();
    videoPortTypes   = _cachedVideoPortTypes;
    videoPortConfigs = _cachedVideoPortConfigs;
    _lock.Unlock();

    for (size_t i = 0; i < videoPortConfigs.size(); ++i) {
        const VideoPortPortConfig& pc = videoPortConfigs[i];

        std::string typeName;
        for (size_t j = 0; j < videoPortTypes.size(); ++j) {
            if (videoPortTypes[j].typeId == pc.videoPortType) {
                typeName = videoPortTypes[j].name;
                break;
            }
        }
        const std::string name = BuildVideoPortName(typeName, pc.videoPortIndex);
        if (EqualsIgnoreCase(name, portName)) {
            connectedAudioType  = pc.connectedAudioPortType;
            connectedAudioIndex = pc.connectedAudioPortIndex;
            return true;
        }
    }
    return false;
}

bool DeviceSettingsConfig::GetVideoPortTypeConfig(VideoPortType typeId, VideoPortTypeConfig& cfg) const
{
    std::vector<VideoPortTypeConfig> videoPortTypes;
    _lock.Lock();
    videoPortTypes = _cachedVideoPortTypes;
    _lock.Unlock();

    for (size_t i = 0; i < videoPortTypes.size(); ++i) {
        if (videoPortTypes[i].typeId == typeId) {
            cfg = videoPortTypes[i];
            return true;
        }
    }
    return false;
}

bool DeviceSettingsConfig::ResolveVideoPortEntryByName(const std::string& requestedPort,
                                                   VideoPortEntry& resolvedEntry) const
{
    std::vector<VideoPortEntry> entries;
    if (!BuildVideoPortEntries(entries)) {
        return false;
    }

    for (size_t i = 0; i < entries.size(); ++i) {
        const VideoPortEntry& e = entries[i];
        if (EqualsIgnoreCase(e.name, requestedPort) ||
            ((e.index == 0) && !e.typeName.empty() && EqualsIgnoreCase(e.typeName, requestedPort))) {
            resolvedEntry = e;
            return true;
        }
    }
    return false;
}

std::vector<VideoPortResolution> DeviceSettingsConfig::GetCachedResolutions() const
{
    _lock.Lock();
    std::vector<VideoPortResolution> res = _cachedVideoPortResolutions;
    _lock.Unlock();
    return res;
}

// ============================================================================
// Audio queries
// ============================================================================

bool DeviceSettingsConfig::BuildAudioPortEntries(std::vector<AudioPortEntry>& entries) const
{
    entries.clear();

    std::vector<AudioPortConfigInfo> audioPortConfigs;
    _lock.Lock();
    audioPortConfigs = _cachedAudioPortConfigs;
    _lock.Unlock();

    for (size_t i = 0; i < audioPortConfigs.size(); ++i) {
        const AudioPortConfigInfo& pc = audioPortConfigs[i];
        AudioPortEntry entry;
        entry.type  = pc.audioPortType;
        entry.index = pc.audioPortIndex;
        entry.name  = BuildAudioPortName(pc.audioPortType, pc.audioPortIndex);
        entries.push_back(entry);
    }
    return !entries.empty();
}

std::string DeviceSettingsConfig::GetDefaultAudioPortName() const
{
    // Mirrors device::Host::getDefaultAudioPortName():
    //   Preference order: HDMI0 or SPEAKER0 > first port.
    std::vector<AudioPortEntry> entries;
    if (!BuildAudioPortEntries(entries)) {
        return std::string("HDMI0");
    }

    std::string defaultName = entries[0].name;
    bool found = false;

    for (size_t i = 0; i < entries.size() && !found; ++i) {
        const std::string& n = entries[i].name;
        if (n.find("HDMI0") != std::string::npos || n.find("SPEAKER0") != std::string::npos) {
            defaultName = n;
            found = true;
        }
    }
    return defaultName;
}

bool DeviceSettingsConfig::GetAudioTypeConfig(int32_t typeId, AudioTypeConfigInfo& cfg) const
{
    std::vector<AudioTypeConfigInfo> audioTypes;
    _lock.Lock();
    audioTypes = _cachedAudioTypeConfigs;
    _lock.Unlock();

    for (size_t i = 0; i < audioTypes.size(); ++i) {
        if (audioTypes[i].typeId == typeId) {
            cfg = audioTypes[i];
            return true;
        }
    }
    return false;
}

// ============================================================================
// VideoDevice queries
// ============================================================================

std::vector<VideoDeviceConfigInfo> DeviceSettingsConfig::GetVideoDeviceConfigs() const
{
    _lock.Lock();
    std::vector<VideoDeviceConfigInfo> devices = _cachedVideoDeviceConfigs;
    _lock.Unlock();
    return devices;
}

bool DeviceSettingsConfig::GetVideoDeviceConfig(int32_t index, VideoDeviceConfigInfo& cfg) const
{
    _lock.Lock();
    const bool valid = (index >= 0) && (static_cast<size_t>(index) < _cachedVideoDeviceConfigs.size());
    if (valid) {
        cfg = _cachedVideoDeviceConfigs[static_cast<size_t>(index)];
    }
    _lock.Unlock();
    return valid;
}

size_t DeviceSettingsConfig::GetVideoDeviceCount() const
{
    _lock.Lock();
    const size_t count = _cachedVideoDeviceConfigs.size();
    _lock.Unlock();
    return count;
}

// ============================================================================
// FPD queries
// ============================================================================

std::vector<FPDIndicatorConfig> DeviceSettingsConfig::GetFPDIndicators() const
{
    _lock.Lock();
    std::vector<FPDIndicatorConfig> v = _cachedFPDIndicators;
    _lock.Unlock();
    return v;
}

std::vector<FPDColorConfig> DeviceSettingsConfig::GetFPDColors() const
{
    _lock.Lock();
    std::vector<FPDColorConfig> v = _cachedFPDColors;
    _lock.Unlock();
    return v;
}

std::vector<FPDTextDisplayConfig> DeviceSettingsConfig::GetFPDTextDisplays() const
{
    _lock.Lock();
    std::vector<FPDTextDisplayConfig> v = _cachedFPDTextDisplays;
    _lock.Unlock();
    return v;
}

std::vector<FPDColorBinding> DeviceSettingsConfig::GetFPDColorBindings() const
{
    _lock.Lock();
    std::vector<FPDColorBinding> v = _cachedFPDColorBindings;
    _lock.Unlock();
    return v;
}

bool DeviceSettingsConfig::GetFPDIndicatorById(int32_t id, FPDIndicatorConfig& cfg) const
{
    std::vector<FPDIndicatorConfig> indicators;
    _lock.Lock();
    indicators = _cachedFPDIndicators;
    _lock.Unlock();

    for (size_t i = 0; i < indicators.size(); ++i) {
        if (indicators[i].id == id) {
            cfg = indicators[i];
            return true;
        }
    }
    return false;
}

bool DeviceSettingsConfig::GetFPDTextDisplayByName(const std::string& name, FPDTextDisplayConfig& cfg) const
{
    std::vector<FPDTextDisplayConfig> textDisplays;
    _lock.Lock();
    textDisplays = _cachedFPDTextDisplays;
    _lock.Unlock();

    for (size_t i = 0; i < textDisplays.size(); ++i) {
        if (EqualsIgnoreCase(textDisplays[i].name, name)) {
            cfg = textDisplays[i];
            return true;
        }
    }
    return false;
}

// ============================================================================
// Internal utilities
// ============================================================================

bool DeviceSettingsConfig::EqualsIgnoreCase(const std::string& lhs, const std::string& rhs)
{
    return (lhs.size() == rhs.size()) &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                      [](char a, char b) {
                          return std::tolower(static_cast<unsigned char>(a)) ==
                                 std::tolower(static_cast<unsigned char>(b));
                      });
}

std::string DeviceSettingsConfig::BuildVideoPortName(const std::string& typeName, int32_t index)
{
    if (typeName.empty()) {
        return std::string("VIDEO") + std::to_string(index);
    }
    return typeName + std::to_string(index);
}

std::string DeviceSettingsConfig::BuildAudioPortName(AudioPortType portType, int32_t index)
{
    switch (portType) {
    case AudioPortType::AUDIO_PORT_TYPE_HDMI:
        return std::string("HDMI") + std::to_string(index);
    case AudioPortType::AUDIO_PORT_TYPE_SPDIF:
        return std::string("SPDIF") + std::to_string(index);
    case AudioPortType::AUDIO_PORT_TYPE_LR:
        return std::string("LR") + std::to_string(index);
    case AudioPortType::AUDIO_PORT_TYPE_SPEAKER:
        return std::string("SPEAKER") + std::to_string(index);
    case AudioPortType::AUDIO_PORT_TYPE_HDMIARC:
        return std::string("HDMIARC") + std::to_string(index);
    case AudioPortType::AUDIO_PORT_TYPE_HEADPHONE:
        return std::string("HEADPHONE") + std::to_string(index);
    default:
        return std::string("AUDIO") + std::to_string(index);
    }
}

} // namespace Plugin
} // namespace WPEFramework
