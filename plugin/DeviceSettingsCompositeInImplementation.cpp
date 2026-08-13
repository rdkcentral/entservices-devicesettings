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

#include "DeviceSettingsCompositeInImplementation.h"

#include <syscall.h>

using namespace std;

namespace WPEFramework {
namespace Plugin {

    DeviceSettingsCompositeInImpl::DeviceSettingsCompositeInImpl() : 
        _CompositeInNotifications(),
        _apiLock(),
        _callbackLock(),
        _compositeIn(CompositeIn::Create<dCompositeInImpl>(*this))
    {
        DSLOG_INFO("Constructor - Instance Address: %p", this);
    }

    DeviceSettingsCompositeInImpl::~DeviceSettingsCompositeInImpl() {
        DSLOG_INFO("Destructor - Instance Address: %p", this);
    }

    template<typename Func, typename... Args>
    void DeviceSettingsCompositeInImpl::dispatchCompositeInEvent(Func notifyFunc, Args&&... args) {
        DSLOG_INFO(">>");
        _callbackLock.Lock();
        for (auto& notification : _CompositeInNotifications) {
            auto start = std::chrono::steady_clock::now();
            (notification->*notifyFunc)(std::forward<Args>(args)...);
            auto elapsed = std::chrono::steady_clock::now() - start;
            DSLOG_INFO("client %p took %" PRId64 "ms to process ICompositeIn event", notification, std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
        }
        _callbackLock.Unlock();
        DSLOG_INFO("<<");
    }

    template <typename T>
    Core::hresult DeviceSettingsCompositeInImpl::Register(std::list<T*>& list, T* notification)
    {
        uint32_t status = Core::ERROR_GENERAL;
        ASSERT(nullptr != notification);

        _callbackLock.Lock();
        // Make sure we can't register the same notification callback multiple times
        if (std::find(list.begin(), list.end(), notification) == list.end()) {
            list.push_back(notification);
            notification->AddRef();
            status = Core::ERROR_NONE;
        } else {
            DSLOG_WARN("Notification %p already registered - skipping", notification);
        }
        _callbackLock.Unlock();

        return status;
    }

    template <typename T>
    Core::hresult DeviceSettingsCompositeInImpl::Unregister(std::list<T*>& list, const T* notification)
    {
        uint32_t status = Core::ERROR_GENERAL;
        ASSERT(nullptr != notification);
        _callbackLock.Lock();

        // Make sure we can't unregister the same notification callback multiple times
        auto itr = std::find(list.begin(), list.end(), notification);
        if (itr != list.end()) {
            (*itr)->Release();
            list.erase(itr);
            status = Core::ERROR_NONE;
        }

        _callbackLock.Unlock();
        return status;
    }

    Core::hresult DeviceSettingsCompositeInImpl::Register(Exchange::IDeviceSettingsCompositeIn::INotification* notification)
    {
        Core::hresult errorCode = Register(_CompositeInNotifications, notification);
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_ERR("ICompositeIn %p, errorCode: %u", notification, errorCode);
        } else {
            DSLOG_INFO("ICompositeIn %p registered successfully", notification);
        }
        return errorCode;
    }

    Core::hresult DeviceSettingsCompositeInImpl::Unregister(Exchange::IDeviceSettingsCompositeIn::INotification* notification)
    {
        Core::hresult errorCode = Unregister(_CompositeInNotifications, notification);
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_ERR("ICompositeIn %p, errorcode: %u", notification, errorCode);
        } else {
            DSLOG_INFO("ICompositeIn %p unregistered successfully", notification);
        }
        return errorCode;
    }

    // CompositeIn::INotification interface implementations (called by DS HAL)
    void DeviceSettingsCompositeInImpl::OnCompositeInHotPlug(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort port, const bool isConnected)
    {
        DSLOG_INFO("DS HAL OnCompositeInHotPlug event: port=%d, isConnected=%s", static_cast<int>(port), isConnected ? "true" : "false");
        
        // Port already converted to WPE type at HAL layer - direct dispatch
        dispatchCompositeInEvent(&Exchange::IDeviceSettingsCompositeIn::INotification::OnCompositeInHotPlug, port, isConnected);
    }

    void DeviceSettingsCompositeInImpl::OnCompositeInSignalStatus(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort port, const Exchange::IDeviceSettingsCompositeIn::CompositeInSignalStatus signalStatus)
    {
        DSLOG_INFO("DS HAL OnCompositeInSignalStatus event: port=%d, signalStatus=%d", static_cast<int>(port), static_cast<int>(signalStatus));
        
        // Types already converted to WPE types at HAL layer - direct dispatch
        dispatchCompositeInEvent(&Exchange::IDeviceSettingsCompositeIn::INotification::OnCompositeInSignalStatus, port, signalStatus);
    }

    void DeviceSettingsCompositeInImpl::OnCompositeInStatus(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort activePort, const bool isPresented)
    {
        DSLOG_INFO("DS HAL OnCompositeInStatus event: activePort=%d, isPresented=%s", static_cast<int>(activePort), isPresented ? "true" : "false");
        
        // Port already converted to WPE type at HAL layer - direct dispatch
        dispatchCompositeInEvent(&Exchange::IDeviceSettingsCompositeIn::INotification::OnCompositeInStatus, activePort, isPresented);
    }

    void DeviceSettingsCompositeInImpl::OnCompositeInVideoModeUpdate(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort activePort, const Exchange::IDeviceSettingsCompositeIn::DisplayVideoPortResolution videoResolution)
    {
        DSLOG_INFO("DS HAL OnCompositeInVideoModeUpdate event: activePort=%d", static_cast<int>(activePort));
        
        // Types already converted to WPE types at HAL layer - direct dispatch
        dispatchCompositeInEvent(&Exchange::IDeviceSettingsCompositeIn::INotification::OnCompositeInVideoModeUpdate, activePort, videoResolution);
    }

    // CompositeIn interface method implementations called by DeviceSettingsImp (delegate to _compositeIn)
    uint32_t DeviceSettingsCompositeInImpl::GetNrOfCompositeInputs(int32_t &nrCompositeInputs)
    {
        uint32_t result = _compositeIn.GetNrOfCompositeInputs(nrCompositeInputs);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded: nrCompositeInputs=%d", nrCompositeInputs);
        } else {
            DSLOG_ERR("failed: error=%u", result);
        }
        return result;
    }

    uint32_t DeviceSettingsCompositeInImpl::GetCompositeInStatus(CompositeInStatus &status)
    {
        uint32_t result = _compositeIn.GetCompositeInStatus(status);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded: activePort=%d, isPresented=%s",
                    static_cast<int>(status.activePort), status.isPresented ? "true" : "false");
        } else {
            DSLOG_ERR("failed: error=%u", result);
        }
        return result;
    }

    uint32_t DeviceSettingsCompositeInImpl::SelectCompositeInPort(const CompositeInPort port)
    {
        uint32_t result = _compositeIn.SelectCompositeInPort(port);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded: port=%d", static_cast<int>(port));
        } else {
            DSLOG_ERR("failed: port=%d, error=%u", static_cast<int>(port), result);
        }
        return result;
    }

    uint32_t DeviceSettingsCompositeInImpl::ScaleCompositeInVideo(const CompositeInVideoRectangle videoRect)
    {
        uint32_t result = _compositeIn.ScaleCompositeInVideo(videoRect);
        if (result == Core::ERROR_NONE) {
            DSLOG_INFO("succeeded: x=%d, y=%d, width=%d, height=%d",
                    videoRect.x, videoRect.y, videoRect.width, videoRect.height);
        } else {
            DSLOG_ERR("failed: error=%u", result);
        }
        return result;
    }



} // namespace Plugin
} // namespace WPEFramework