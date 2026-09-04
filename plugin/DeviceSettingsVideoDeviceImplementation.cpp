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

#include "DeviceSettingsVideoDeviceImplementation.h"

#include <syscall.h>
#include <vector>

#include "DeviceSettingsHALConfig.h"

using namespace std;

namespace WPEFramework {
namespace Plugin {

    DeviceSettingsVideoDeviceImpl::DeviceSettingsVideoDeviceImpl() : 
        _VideoDeviceNotifications(),
        _apiLock(),
        _callbackLock(),
        _videoDevice(VideoDevice::Create(*this))
    {
        DSLOG_INFO("Constructor - Instance Address: %p", this);
    }

    DeviceSettingsVideoDeviceImpl::~DeviceSettingsVideoDeviceImpl() {
        DSLOG_INFO("Destructor - Instance Address: %p", this);
    }

    template<typename Func, typename... Args>
    void DeviceSettingsVideoDeviceImpl::dispatchVideoDeviceEvent(Func notifyFunc, Args&&... args) {
        DSLOG_INFO(">>");
        std::vector<std::pair<string, Exchange::IDeviceSettingsVideoDevice::INotification*>> notifications;
        _callbackLock.Lock();
        for (auto& entry : _VideoDeviceNotifications) {
            entry.second->AddRef();
            notifications.push_back(entry);
        }
        _callbackLock.Unlock();

        DSLOG_INFO(">>> Dispatching VideoDevice event to %zu clients", notifications.size());

        for (auto& entry : notifications) {
            const string& clientName = entry.first;
            auto* notification = entry.second;
            auto start = std::chrono::steady_clock::now();
            (notification->*notifyFunc)(std::forward<Args>(args)...);
            auto elapsed = std::chrono::steady_clock::now() - start;
            DSLOG_INFO("client '%s' took %" PRId64 "ms to process IVideoDevice event", clientName.c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
            notification->Release();
        }
        DSLOG_INFO("<<");
    }

    template <typename T>
    Core::hresult DeviceSettingsVideoDeviceImpl::Register(std::list<std::pair<string, T*>>& list, const string& clientName, T* notification)
    {
        uint32_t status = Core::ERROR_GENERAL;
        ASSERT(nullptr != notification);

        _callbackLock.Lock();
        // Make sure we can't register the same notification callback multiple times
        auto it = std::find_if(list.begin(), list.end(), [notification](const std::pair<string, T*>& p){ return p.second == notification; });
        if (it == list.end()) {
            list.push_back({clientName, notification});
            notification->AddRef();
            status = Core::ERROR_NONE;
        } else {
            DSLOG_WARN("Notification %p already registered - skipping", notification);
        }
        _callbackLock.Unlock();

        return status;
    }

    template <typename T>
    Core::hresult DeviceSettingsVideoDeviceImpl::Unregister(std::list<std::pair<string, T*>>& list, const T* notification)
    {
        uint32_t status = Core::ERROR_GENERAL;
        ASSERT(nullptr != notification);
        _callbackLock.Lock();

        // Make sure we can't unregister the same notification callback multiple times
        auto itr = std::find_if(list.begin(), list.end(), [notification](const std::pair<string, T*>& p){ return p.second == notification; });
        if (itr != list.end()) {
            itr->second->Release();
            list.erase(itr);
            status = Core::ERROR_NONE;
        }

        _callbackLock.Unlock();
        return status;
    }

    Core::hresult DeviceSettingsVideoDeviceImpl::Register(const string& clientName, Exchange::IDeviceSettingsVideoDevice::INotification* notification)
    {
        Core::hresult errorCode = Register(_VideoDeviceNotifications, clientName, notification);
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_ERR("IVideoDevice %p [%s], errorCode: %u", notification, clientName.c_str(), errorCode);
        } else {
            DSLOG_INFO("IVideoDevice %p [%s] registered successfully", notification, clientName.c_str());
        }
        return errorCode;
    }

    Core::hresult DeviceSettingsVideoDeviceImpl::Unregister(Exchange::IDeviceSettingsVideoDevice::INotification* notification)
    {
        Core::hresult errorCode = Unregister(_VideoDeviceNotifications, notification);
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_ERR("IVideoDevice %p, errorcode: %u", notification, errorCode);
        } else {
            DSLOG_INFO("IVideoDevice %p unregistered successfully", notification);
        }
        return errorCode;
    }

    // VideoDevice::INotification interface implementations (called by DS HAL)
    void DeviceSettingsVideoDeviceImpl::OnZoomSettingsChanged(const VideoDeviceZoom zoomSetting)
    {
        DSLOG_INFO("DS HAL OnZoomSettingsChanged event: zoomSetting=%d", static_cast<int>(zoomSetting));
        dispatchVideoDeviceEvent(&Exchange::IDeviceSettingsVideoDevice::INotification::OnZoomSettingsChanged, zoomSetting);
    }

    void DeviceSettingsVideoDeviceImpl::OnDisplayFrameratePreChange(const string frameRate)
    {
        DSLOG_INFO("DS HAL OnDisplayFrameratePreChange event: frameRate=%s", frameRate.c_str());
        dispatchVideoDeviceEvent(&Exchange::IDeviceSettingsVideoDevice::INotification::OnDisplayFrameratePreChange, frameRate);
    }

    void DeviceSettingsVideoDeviceImpl::OnDisplayFrameratePostChange(const string frameRate)
    {
        DSLOG_INFO("DS HAL OnDisplayFrameratePostChange event: frameRate=%s", frameRate.c_str());
        dispatchVideoDeviceEvent(&Exchange::IDeviceSettingsVideoDevice::INotification::OnDisplayFrameratePostChange, frameRate);
    }

    // VideoDevice interface method implementations called by DeviceSettingsImp 
    uint32_t DeviceSettingsVideoDeviceImpl::GetVideoDeviceHandle(const int32_t index, int32_t &handle)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.GetVideoDeviceHandle(index, handle);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded: index=%d, handle=%d", index, handle);
        } else {
            DSLOG_ERR("failed: index=%d, error=%u", index, result);
        }
        return result;
    }

    uint32_t DeviceSettingsVideoDeviceImpl::SetVideoDeviceDFC(const int32_t handle, const VideoDeviceZoom zoomSetting)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.SetVideoDeviceDFC(handle, zoomSetting);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded for handle: %d, zoomSetting: %d", handle, static_cast<int>(zoomSetting));
        } else {
            DSLOG_ERR("failed for handle: %d, zoomSetting: %d, error: %u", handle, static_cast<int>(zoomSetting), result);
        }
        return result;
    }

    uint32_t DeviceSettingsVideoDeviceImpl::GetVideoDeviceDFC(const int32_t handle, VideoDeviceZoom &zoomSetting)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.GetVideoDeviceDFC(handle, zoomSetting);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded for handle: %d, zoomSetting: %d", handle, static_cast<int>(zoomSetting));
        } else {
            DSLOG_ERR("failed for handle: %d, error: %u", handle, result);
        }
        return result;
    }

    uint32_t DeviceSettingsVideoDeviceImpl::GetHDRCapabilities(const int32_t handle, int32_t &capabilities)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.GetHDRCapabilities(handle, capabilities);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded for handle: %d, capabilities: 0x%x", handle, capabilities);
        } else {
            DSLOG_ERR("failed for handle: %d, error: %u", handle, result);
        }
        return result;
    }

    uint32_t DeviceSettingsVideoDeviceImpl::GetSupportedVideoCodingFormats(const int32_t handle, int32_t &supportedFormats)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.GetSupportedVideoCodingFormats(handle, supportedFormats);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded for handle: %d, supportedFormats: 0x%x", handle, supportedFormats);
        } else {
            DSLOG_ERR("failed for handle: %d, error: %u", handle, result);
        }
        return result;
    }

    uint32_t DeviceSettingsVideoDeviceImpl::GetCodecInfo(const int32_t handle, const VideoDeviceCodec videoCodec, IDeviceSettingsVideoCodecProfileSupportIterator*& codecInfo)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.GetCodecInfo(handle, videoCodec, codecInfo);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded for handle: %d, videoCodec: %d", handle, static_cast<int>(videoCodec));
        } else {
            DSLOG_ERR("failed for handle: %d, videoCodec: %d, error: %u", handle, static_cast<int>(videoCodec), result);
        }
        return result;
    }

    uint32_t DeviceSettingsVideoDeviceImpl::DisableHDR(const int32_t handle, const bool disable)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.DisableHDR(handle, disable);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded for handle: %d, disable: %s", handle, disable ? "true" : "false");
        } else {
            DSLOG_ERR("failed for handle: %d, disable: %s, error: %u", handle, disable ? "true" : "false", result);
        }
        return result;
    }

    uint32_t DeviceSettingsVideoDeviceImpl::SetFRFMode(const int32_t handle, const int32_t frfmode)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.SetFRFMode(handle, frfmode);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded for handle: %d, frfmode: %d", handle, frfmode);
        } else {
            DSLOG_ERR("failed for handle: %d, frfmode: %d, error: %u", handle, frfmode, result);
        }
        return result;
    }

    uint32_t DeviceSettingsVideoDeviceImpl::GetFRFMode(const int32_t handle, int32_t &frfmode)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.GetFRFMode(handle, frfmode);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded for handle: %d, frfmode: %d", handle, frfmode);
        } else {
            DSLOG_ERR("failed for handle: %d, error: %u", handle, result);
        }
        return result;
    }

    uint32_t DeviceSettingsVideoDeviceImpl::GetCurrentDisplayFrameRate(const int32_t handle, string &framerate)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.GetCurrentDisplayFrameRate(handle, framerate);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded for handle: %d, framerate: %s", handle, framerate.c_str());
        } else {
            DSLOG_ERR("failed for handle: %d, error: %u", handle, result);
        }
        return result;
    }

    uint32_t DeviceSettingsVideoDeviceImpl::SetDisplayFrameRate(const int32_t handle, const string framerate)
    {
        uint32_t result = Core::ERROR_GENERAL;
        result = _videoDevice.SetDisplayFrameRate(handle, framerate);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded for handle: %d, framerate: %s", handle, framerate.c_str());
        } else {
            DSLOG_ERR("failed for handle: %d, framerate: %s, error: %u", handle, framerate.c_str(), result);
        }
        return result;
    }

    void DeviceSettingsVideoDeviceImpl::getCachedConfigs(
        std::vector<Exchange::IDeviceSettings::VideoDeviceConfigInfo>& videoConfigs) const
    {
        _apiLock.Lock();

        videoConfigs.reserve(_cachedVideoDeviceConfigs.size());
        for (const auto& src : _cachedVideoDeviceConfigs) {
            videoConfigs.push_back({src.numSupportedDFCs, src.supportedDFCsMask,
                static_cast<int32_t>(src.defaultDFC)});
        }

        _apiLock.Unlock();
    }

} // namespace Plugin
} // namespace WPEFramework