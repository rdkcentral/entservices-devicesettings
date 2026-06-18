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

#include "Module.h"
#include "DeviceSettingsTypes.h"

namespace WPEFramework {
namespace Plugin {

class DeviceSettingsImp;

/**
 * @brief Central cache and accessor for all static device configuration data.
 *
 * Replaces direct use of lib32-devicesettings ds/ singletons:
 *   - VideoOutputPortConfig::getInstance() / Host::getVideoOutputPorts()
 *   - AudioOutputPortConfig::getInstance() / Host::getAudioOutputPorts()
 *   - VideoDeviceConfig::getInstance()      / Host::getVideoDevices()
 *   - FrontPanelConfig::getInstance()
 *
 * Usage:
 *   1. Call Refresh() once after DeviceSettingsImp is available.
 *   2. Use query methods in place of legacy device:: wrappers.
 */
class DeviceSettingsConfig {
public:
    // -----------------------------------------------------------------------
    // Port entry helpers (for DSPwrEventListener and similar power paths)
    // -----------------------------------------------------------------------
    struct VideoPortEntry {
        std::string name;       ///< Constructed name, e.g. "HDMI0"
        std::string typeName;   ///< Type string from VideoPortTypeConfig
        VideoPortType type;     ///< DS_VIDEO_PORT_TYPE_* enum value
        int32_t index;
    };

    struct AudioPortEntry {
        std::string name;       ///< Constructed name, e.g. "SPEAKER0"
        AudioPortType type;     ///< AUDIO_PORT_TYPE_* enum value
        int32_t index;
    };

    // -----------------------------------------------------------------------
    // Refresh (populates all four configuration caches)
    // -----------------------------------------------------------------------

    /**
     * @brief Populate all four configuration caches from the DeviceSettings
     *        plugin.  Should be called once after the plugin is initialised.
     *        Internally delegates to the four specialised methods below.
     */
    bool Refresh(DeviceSettingsImp* deviceSettings);

    /** @brief Returns true if none of the four caches have been populated. */
    bool IsCacheEmpty() const;

    // -----------------------------------------------------------------------
    // VideoPort configuration  (mirrors GetVideoPortConfig)
    // -----------------------------------------------------------------------

    /** @brief Build a flat list of all video-output port entries from cache. */
    bool BuildVideoPortEntries(std::vector<VideoPortEntry>& entries) const;

    /**
     * @brief Return the name of the preferred default video port.
     *        Logic mirrors device::Host::getDefaultVideoPortName():
     *          HDMI0 > INTERNAL0 > first enumerated port.
     */
    std::string GetDefaultVideoPortName() const;

    /** @brief True if any HDMI-type video-output port exists in cache. */
    bool IsHDMIOutPortPresent() const;

    /**
     * @brief Return the default resolution string for a given port by name.
     * @param portName  e.g. "HDMI0"
     * @return default resolution string (e.g. "1080p60"), or empty if not found.
     */
    std::string GetVideoPortDefaultResolution(const std::string& portName) const;

    /**
     * @brief Look up the connected audio port identifiers for a video port.
     * @param portName          e.g. "HDMI0"
     * @param connectedAudioType  [out] connected audio port type int32_t
     * @param connectedAudioIndex [out] connected audio port index
     * @return true if the port was found in cache.
     */
    bool GetVideoPortConnectedAudioPort(const std::string& portName,
                                        int32_t& connectedAudioType,
                                        int32_t& connectedAudioIndex) const;

    /**
     * @brief Find a VideoPortTypeConfig by VideoPortType enum value.
     * @param typeId   e.g. DS_VIDEO_PORT_TYPE_HDMI
     * @param cfg      [out] matching config struct
     * @return true if found.
     */
    bool GetVideoPortTypeConfig(VideoPortType typeId, VideoPortTypeConfig& cfg) const;

    /**
     * @brief Resolve a video port entry by name (or type-name alias).
     *        Case-insensitive; also matches bare type name (e.g. "HDMI").
     */
    bool ResolveVideoPortEntryByName(const std::string& requestedPort,
                                     VideoPortEntry& resolvedEntry) const;

    /**
     * @brief Return cached global resolution list (built from VideoPortConfig).
     */
    std::vector<VideoPortResolution> GetCachedResolutions() const;

    // -----------------------------------------------------------------------
    // Audio configuration  (mirrors GetAudioConfig / GetAudioPortConfig)
    // -----------------------------------------------------------------------

    /** @brief Build a flat list of all audio-output port entries from cache. */
    bool BuildAudioPortEntries(std::vector<AudioPortEntry>& entries) const;

    /**
     * @brief Return the name of the preferred default audio port.
     *        Logic mirrors device::Host::getDefaultAudioPortName():
     *          HDMI0 > SPEAKER0 > first enumerated port.
     */
    std::string GetDefaultAudioPortName() const;

    /**
     * @brief Find an AudioTypeConfigInfo by numeric typeId.
     * @param typeId  numeric type id from AudioTypeConfigInfo::typeId
     * @param cfg     [out] matching config struct
     * @return true if found.
     */
    bool GetAudioTypeConfig(int32_t typeId, AudioTypeConfigInfo& cfg) const;

    // -----------------------------------------------------------------------
    // VideoDevice configuration  (mirrors GetVideoDeviceConfig)
    // -----------------------------------------------------------------------

    /**
     * @brief Return all cached VideoDeviceConfigInfo entries.
     *        Mirrors device::VideoDeviceConfig::getDevices().
     */
    std::vector<VideoDeviceConfigInfo> GetVideoDeviceConfigs() const;

    /**
     * @brief Get the VideoDeviceConfigInfo at a given index.
     * @param index  0-based device index
     * @param cfg    [out] device config
     * @return true if the index is valid.
     */
    bool GetVideoDeviceConfig(int32_t index, VideoDeviceConfigInfo& cfg) const;

    /** @brief Return the number of cached video devices. */
    size_t GetVideoDeviceCount() const;

    // -----------------------------------------------------------------------
    // FrontPanel configuration  (mirrors GetFrontPanelConfig)
    // -----------------------------------------------------------------------

    /**
     * @brief Return all cached FPD indicator configs.
     *        Mirrors device::FrontPanelConfig::getIndicators().
     */
    std::vector<FPDIndicatorConfig> GetFPDIndicators() const;

    /**
     * @brief Return all cached FPD color configs.
     *        Mirrors device::FrontPanelConfig::getColors().
     */
    std::vector<FPDColorConfig> GetFPDColors() const;

    /**
     * @brief Return all cached FPD text display configs.
     *        Mirrors device::FrontPanelConfig::getTextDisplays().
     */
    std::vector<FPDTextDisplayConfig> GetFPDTextDisplays() const;

    /**
     * @brief Return all cached FPD color-binding entries.
     */
    std::vector<FPDColorBinding> GetFPDColorBindings() const;

    /**
     * @brief Find an FPDIndicatorConfig by indicator id.
     * @param id   indicator id (from FPDIndicatorConfig::id)
     * @param cfg  [out] matching config
     * @return true if found.
     */
    bool GetFPDIndicatorById(int32_t id, FPDIndicatorConfig& cfg) const;

    /**
     * @brief Find an FPDTextDisplayConfig by display name.
     * @param name  display name (from FPDTextDisplayConfig::name)
     * @param cfg   [out] matching config
     * @return true if found.
     */
    bool GetFPDTextDisplayByName(const std::string& name, FPDTextDisplayConfig& cfg) const;

private:
    // -----------------------------------------------------------------------
    // Four individual refresh methods — one per plugin config API
    // -----------------------------------------------------------------------

    /** Calls DeviceSettingsImp::GetVideoPortConfig and stores results. */
    bool RefreshVideoPortConfig(DeviceSettingsImp* deviceSettings);

    /**
     * Calls DeviceSettingsImp::GetAudioConfig (bulk iterator) and stores
     * both AudioTypeConfigInfo and AudioPortConfigInfo caches.
     */
    bool RefreshAudioConfig(DeviceSettingsImp* deviceSettings);

    /** Calls DeviceSettingsImp::GetVideoDeviceConfig and stores results. */
    bool RefreshVideoDeviceConfig(DeviceSettingsImp* deviceSettings);

    /** Calls DeviceSettingsImp::GetFrontPanelConfig and stores results. */
    bool RefreshFrontPanelConfig(DeviceSettingsImp* deviceSettings);

    // -----------------------------------------------------------------------
    // Internal utilities
    // -----------------------------------------------------------------------
    static bool EqualsIgnoreCase(const std::string& lhs, const std::string& rhs);
    static std::string BuildVideoPortName(const std::string& typeName, int32_t index);
    static std::string BuildAudioPortName(AudioPortType portType, int32_t index);

private:
    mutable Core::CriticalSection _lock;

    // --- VideoPort cache (from GetVideoPortConfig) ---
    std::vector<VideoPortTypeConfig>  _cachedVideoPortTypes;
    std::vector<VideoPortPortConfig>  _cachedVideoPortConfigs;
    std::vector<VideoPortResolution>  _cachedVideoPortResolutions;

    // --- Audio cache (from GetAudioConfig) ---
    std::vector<AudioTypeConfigInfo>  _cachedAudioTypeConfigs;
    std::vector<AudioPortConfigInfo>  _cachedAudioPortConfigs;

    // --- VideoDevice cache (from GetVideoDeviceConfig) ---
    std::vector<VideoDeviceConfigInfo> _cachedVideoDeviceConfigs;

    // --- FPD cache (from GetFrontPanelConfig) ---
    std::vector<FPDColorConfig>       _cachedFPDColors;
    std::vector<FPDIndicatorConfig>   _cachedFPDIndicators;
    std::vector<FPDTextDisplayConfig> _cachedFPDTextDisplays;
    std::vector<FPDColorBinding>      _cachedFPDColorBindings;
};

} // namespace Plugin
} // namespace WPEFramework
