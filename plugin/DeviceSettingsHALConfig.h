/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

/**
 * @file DeviceSettingsHALConfig.h
 * @brief Shared HAL configuration loading for DeviceSettings components.
 *
 * FPD, Audio, and VideoPort all use the same dlopen/dlsym/dlclose pattern to
 * read static configuration tables from the HAL shared library at startup.
 * This header centralises the public interface for those loaders so the three
 * implementation files do not each carry a private copy of the same code.
 *
 * Include this header AFTER Module.h (or after any header that includes
 * Module.h) in a DeviceSettings plugin compilation unit.
 */

#include "DeviceSettingsTypes.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <vector>

namespace DeviceSettingsHAL {

    // ─── Front Panel Display ───────────────────────────────────────────────────

    /**
     * Load FPD configuration from the HAL shared library and populate the
     * supplied vectors with deep-copied, heap-owned data.  The HAL library
     * handle is closed internally once the copy is complete.
     */
    void PopulateFPDConfig(
        std::vector<FPDColorConfig>& colors,
        std::vector<FPDIndicatorConfig>& indicators,
        std::vector<FPDTextDisplayConfig>& textDisplays,
        std::vector<FPDColorBinding>& colorBindings);

    /**
     * Log a human-readable dump of the FPD config vectors.
     * Gated by /opt/dsMgrDumpDeviceConfigs — no-op when that file is absent.
     */
    void DumpFPDConfig(
        const std::vector<FPDColorConfig>& colors,
        const std::vector<FPDIndicatorConfig>& indicators,
        const std::vector<FPDTextDisplayConfig>& textDisplays,
        const std::vector<FPDColorBinding>& colorBindings);

    // ─── Audio ─────────────────────────────────────────────────────────────────

    void PopulateAudioConfig(
        std::vector<AudioTypeConfigInfo>& audioTypes,
        std::vector<AudioPortConfigInfo>& audioPorts);

    void DumpAudioConfig(
        const std::vector<AudioTypeConfigInfo>& audioTypes,
        const std::vector<AudioPortConfigInfo>& audioPorts);

    // ─── Video Port ────────────────────────────────────────────────────────────

    void PopulateVideoPortConfig(
        std::vector<VideoPortTypeConfig>& videoPortTypes,
        std::vector<VideoPortPortConfig>& videoPorts);

    void PopulateVideoPortResolutionConfig(
        const VideoPortType videoPortType,
        std::vector<VideoPortResolution>& resolutions);

    void DumpVideoPortConfig(
        const std::vector<VideoPortTypeConfig>& videoPortTypes,
        const std::vector<VideoPortPortConfig>& videoPorts,
        const std::vector<VideoPortResolution>& resolutions);

    enum class HandleLookupResult { Found, Unsupported, Failed };

    inline HandleLookupResult HandleLookupStatus(const bool found)
    {
        return found ? HandleLookupResult::Found : HandleLookupResult::Failed;
    }

    inline HandleLookupResult HandleLookupStatus(const HandleLookupResult result)
    {
        return result;
    }

    template <typename Port, typename GetHandle, typename SamePort>
    bool ResolvePortHandle(const int32_t handle, const std::vector<Port>& ports,
        GetHandle getHandle, SamePort samePort, intptr_t& halHandle, Port& resolvedPort)
    {
        const Port* match = nullptr;
        intptr_t matchedHandle = 0;
        for (const auto& port : ports) {
            intptr_t candidate = 0;
            const auto result = HandleLookupStatus(getHandle(port, candidate));
            if (result == HandleLookupResult::Unsupported) {
                continue;
            }
            if (result != HandleLookupResult::Found) {
                return false;
            }
            if (static_cast<int32_t>(candidate) != handle) {
                continue;
            }
            if (match != nullptr
                && (matchedHandle != candidate || !samePort(*match, port))) {
                return false;
            }
            match = &port;
            matchedHandle = candidate;
        }

        if (match == nullptr) {
            return false;
        }
        halHandle = matchedHandle;
        resolvedPort = *match;
        return true;
    }

    struct VideoPortIdentity {
        template <typename Port>
        bool operator()(const Port& first, const Port& second) const
        {
            return first.videoPortType == second.videoPortType
                && first.videoPortIndex == second.videoPortIndex;
        }
    };

    struct AudioPortIdentity {
        template <typename Port>
        bool operator()(const Port& first, const Port& second) const
        {
            return first.audioPortType == second.audioPortType
                && first.audioPortIndex == second.audioPortIndex;
        }
    };

    template <typename Port, typename GetHandle>
    bool ResolveVideoPortHandle(const int32_t handle, const std::vector<Port>& ports,
        GetHandle getHandle, intptr_t& halHandle, Port& resolvedPort)
    {
        return ResolvePortHandle(handle, ports, getHandle, VideoPortIdentity {},
            halHandle, resolvedPort);
    }

    template <typename Port, typename GetHandle>
    bool ResolveAudioPortHandle(const int32_t handle, const std::vector<Port>& ports,
        GetHandle getHandle, intptr_t& halHandle, Port& resolvedPort)
    {
        return ResolvePortHandle(handle, ports, getHandle, AudioPortIdentity {},
            halHandle, resolvedPort);
    }

    template <typename Port, typename SamePort = std::equal_to<Port>>
    class PortHandleCache {
    private:
        struct Entry {
            Port port;
            intptr_t nativeHandle;
        };
        std::vector<Entry> _entries;
        bool _ready = false;

    public:
        void Clear()
        {
            _entries.clear();
            _ready = false;
        }

        bool IsReady() const
        {
            return _ready;
        }

        template <typename GetHandle>
        bool Populate(const std::vector<Port>& ports, GetHandle getHandle)
        {
            Clear();
            if (ports.empty()) {
                return false;
            }
            std::vector<Entry> entries;
            entries.reserve(ports.size());
            for (const auto& port : ports) {
                intptr_t nativeHandle = 0;
                const auto result = HandleLookupStatus(getHandle(port, nativeHandle));
                if (result == HandleLookupResult::Unsupported) {
                    continue;
                }
                if (result != HandleLookupResult::Found) {
                    return false;
                }
                entries.push_back(Entry { port, nativeHandle });
            }
            _entries.swap(entries);
            _ready = true;
            return true;
        }

        bool Resolve(const int32_t handle, intptr_t& halHandle, Port& resolvedPort) const
        {
            Entry resolved {};
            if (!ResolvePortHandle(handle, _entries,
                    [](const Entry& entry, intptr_t& candidate) {
                        candidate = entry.nativeHandle;
                        return true;
                    },
                    [](const Entry& first, const Entry& second) {
                        return SamePort {}(first.port, second.port);
                    }, halHandle, resolved)) {
                return false;
            }
            resolvedPort = resolved.port;
            return true;
        }
    };

    template <typename Port>
    using AudioPortHandleCache = PortHandleCache<Port, AudioPortIdentity>;

    template <typename Port>
    using VideoPortHandleCache = PortHandleCache<Port, VideoPortIdentity>;

    // ─── Video Device ──────────────────────────────────────────────────────────

    void PopulateVideoDeviceConfig(
        std::vector<VideoDeviceConfigInfo>& videoDeviceConfigs);

    void DumpVideoDeviceConfig(
        const std::vector<VideoDeviceConfigInfo>& videoDeviceConfigs);

    inline bool PopulateVideoDeviceIndices(const size_t deviceCount, std::vector<int32_t>& indices)
    {
        indices.clear();
        if (deviceCount > static_cast<size_t>(std::numeric_limits<int32_t>::max())) {
            return false;
        }
        // Legacy VideoDeviceConfig constructs VideoDevice(i) for each config entry.
        indices.reserve(deviceCount);
        for (size_t i = 0; i < deviceCount; ++i) {
            indices.push_back(static_cast<int32_t>(i));
        }
        return true;
    }

    template <typename GetHandle>
    bool ResolveVideoDeviceHandle(const int32_t handle, const size_t deviceCount,
        GetHandle getHandle, intptr_t& halHandle, int32_t& deviceIndex)
    {
        std::vector<int32_t> indices;
        if (!PopulateVideoDeviceIndices(deviceCount, indices)) {
            return false;
        }
        return ResolvePortHandle(handle, indices, getHandle,
            [](const int32_t first, const int32_t second) { return first == second; },
            halHandle, deviceIndex);
    }

} // namespace DeviceSettingsHAL
