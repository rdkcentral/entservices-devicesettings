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

#include "DeviceSettingsFPDImplementation.h"

#include <syscall.h>
#include <vector>

#include "DeviceSettingsHALConfig.h"

using namespace std;

namespace WPEFramework {
namespace Plugin {

    DeviceSettingsFPDImpl::DeviceSettingsFPDImpl()
        : _fpd(FPD::Create(*this))
    {
        DSLOG_INFO("Constructor - Instance Address: %p", this);
    }

    DeviceSettingsFPDImpl::~DeviceSettingsFPDImpl() {
        DSLOG_INFO("Destructor - Instance Address: %p", this);
    }

    template<typename Func, typename... Args>
    void DeviceSettingsFPDImpl::dispatchFPDEvent(Func notifyFunc, Args&&... args) {
        DSLOG_INFO(">>");
        std::vector<std::pair<string, Exchange::IDeviceSettingsFPD::INotification*>> notifications;
        _callbackLock.Lock();
        for (auto& entry : _FPDNotifications) {
            entry.second->AddRef();
            notifications.push_back(entry);
        }
        _callbackLock.Unlock();

        DSLOG_INFO(">>> Dispatching FPD event to %zu clients", notifications.size());

        for (auto& entry : notifications) {
            const string& clientName = entry.first;
            auto* notification = entry.second;
            auto start = std::chrono::steady_clock::now();
            (notification->*notifyFunc)(std::forward<Args>(args)...);
            auto elapsed = std::chrono::steady_clock::now() - start;
            DSLOG_INFO("client '%s' took %" PRId64 "ms to process IFPD event", clientName.c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
            notification->Release();
        }
        DSLOG_INFO("<<");
    }

    template <typename T>
    Core::hresult DeviceSettingsFPDImpl::Register(std::list<std::pair<string, T*>>& list, const string& clientName, T* notification)
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
    Core::hresult DeviceSettingsFPDImpl::Unregister(std::list<std::pair<string, T*>>& list, const T* notification)
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

    Core::hresult DeviceSettingsFPDImpl::Register(const string& clientName, DeviceSettingsFPD::INotification* notification)
    {
        Core::hresult errorCode = Register(_FPDNotifications, clientName, notification);
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_ERR("IFPD %p [%s], errorCode: %u", notification, clientName.c_str(), errorCode);
        } else {
            DSLOG_INFO("IFPD %p [%s] registered successfully", notification, clientName.c_str());
        }
        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::Unregister(DeviceSettingsFPD::INotification* notification)
    {
        Core::hresult errorCode = Unregister(_FPDNotifications, notification);
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_ERR("IFPD %p, errorcode: %u", notification, errorCode);
        } else {
            DSLOG_INFO("IFPD %p unregistered successfully", notification);
        }
        return errorCode;
    }

    // FPD notification implementation
    void DeviceSettingsFPDImpl::OnFPDTimeFormatChanged(const FPDTimeFormat timeFormat)
    {
        DSLOG_INFO("event Received: timeFormat=%d", timeFormat);
        dispatchFPDEvent(&DeviceSettingsFPD::INotification::OnFPDTimeFormatChanged, timeFormat);
    }

    //Depricated
    Core::hresult DeviceSettingsFPDImpl::SetFPDTime(const FPDTimeFormat timeFormat, const uint32_t minutes, const uint32_t seconds) {
        DSLOG_INFO("timeFormat=%d, minutes=%u, seconds=%u", timeFormat, minutes, seconds);
        DSLOG_INFO("SUCCESS - stub implementation completed");
        return Core::ERROR_NONE;
    }

    Core::hresult DeviceSettingsFPDImpl::SetFPDScroll(const uint32_t scrollHoldDuration, const uint32_t nHorizontalScrollIterations, const uint32_t nVerticalScrollIterations) {
        DSLOG_INFO("scrollHoldDuration=%u, horizontal=%u, vertical=%u", scrollHoldDuration, nHorizontalScrollIterations, nVerticalScrollIterations);
        DSLOG_INFO("SUCCESS - stub implementation completed");
        return Core::ERROR_NONE;
    }

    Core::hresult DeviceSettingsFPDImpl::SetFPDTextBrightness(const FPDTextDisplay textDisplay, const uint32_t brightNess) {
        DSLOG_INFO("textDisplay=%d, brightNess=%u", textDisplay, brightNess);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        if (_fpd.SetFPDTextBrightness(textDisplay, brightNess) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - platform call completed");
        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::GetFPDTextBrightness(const FPDTextDisplay textDisplay, uint32_t &brightNess) {
        Core::hresult errorCode = Core::ERROR_GENERAL;
        if (_fpd.GetFPDTextBrightness(textDisplay, brightNess) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - textDisplay=%d, brightNess=%d", textDisplay, brightNess);
        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::EnableFPDClockDisplay(const bool enable) {
        Core::hresult errorCode = Core::ERROR_GENERAL;
        if (_fpd.EnableFPDClockDisplay(enable) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("enable=%s", enable ? "true" : "false");
        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::GetFPDTimeFormat(FPDTimeFormat &fpdTimeFormat) {
        Core::hresult errorCode = Core::ERROR_GENERAL;
        if (_fpd.GetFPDTimeFormat(fpdTimeFormat) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - fpdTimeFormat=%d", fpdTimeFormat);
        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::SetFPDTimeFormat(const FPDTimeFormat fpdTimeFormat) {
        Core::hresult errorCode = Core::ERROR_GENERAL;
        if (_fpd.SetFPDTimeFormat(fpdTimeFormat) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("fpdTimeFormat=%d", fpdTimeFormat);
        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::SetFPDBlink(const FPDIndicator indicator, const uint32_t blinkDuration, const uint32_t blinkIterations) {
        Core::hresult errorCode = Core::ERROR_GENERAL;
        if (_fpd.SetFPDBlink(indicator, blinkDuration, blinkIterations) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("indicator=%d, blinkDuration=%u, blinkIterations=%u", indicator, blinkDuration, blinkIterations);
        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::SetFPDMode(const FPDMode fpdMode) {
        Core::hresult errorCode = Core::ERROR_GENERAL;
        if (_fpd.SetFPDMode(fpdMode) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("fpdMode=%d", fpdMode);
        return errorCode;
    }
    //Depricated

    Core::hresult DeviceSettingsFPDImpl::SetFPDBrightness(const FPDIndicator indicator, const uint32_t brightNess, const bool persist) {
        DSLOG_INFO("indicator=%d, brightNess=%u, persist=%s", indicator, brightNess, persist ? "true" : "false");

        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_fpd.SetFPDBrightness(indicator, brightNess, persist) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }
        
        DSLOG_INFO("SUCCESS - platform call completed");

        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::GetFPDBrightness(const FPDIndicator indicator, uint32_t &brightNess, const bool persist) {

        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_fpd.GetFPDBrightness(indicator, brightNess, persist) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - indicator=%d, brightNess=%d, persist=%s", indicator, brightNess, persist ? "true" : "false");
        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::SetFPDState(const FPDIndicator indicator, const FPDState state) {
        DSLOG_INFO("indicator=%d, state=%d", indicator, state);

        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_fpd.SetFPDState(indicator, state) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }
        
        DSLOG_INFO("SUCCESS - platform call completed");

        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::GetFPDState(const FPDIndicator indicator, FPDState &state) {

        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_fpd.GetFPDState(indicator, state) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - indicator=%d, state=%d", indicator, state);
        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::GetFPDColor(const FPDIndicator indicator, uint32_t &color) {

        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_fpd.GetFPDColor(indicator, color) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - indicator=%d, color=0x%X", indicator, color);
        return errorCode;
    }

    Core::hresult DeviceSettingsFPDImpl::SetFPDColor(const FPDIndicator indicator, const uint32_t color) {
        DSLOG_INFO("indicator=%d, color=0x%X", indicator, color);

        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_fpd.SetFPDColor(indicator, color) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }
        
        DSLOG_INFO("SUCCESS - platform call completed");

        return errorCode;
    }

    void DeviceSettingsFPDImpl::getCachedConfigs(
        std::vector<Exchange::IDeviceSettings::FPDTextDisplayConfig>& textDisplays,
        std::vector<Exchange::IDeviceSettings::FPDIndicatorConfig>& indicators,
        std::vector<Exchange::IDeviceSettings::FPDColorConfig>& colors,
        std::vector<Exchange::IDeviceSettings::FPDColorBinding>& colorBindings) const
    {
        // FPD types are identical in IDeviceSettings — direct assignment, no field-by-field copy
        _apiLock.Lock();
        textDisplays.assign(_cachedTextDisplayConfigs.begin(), _cachedTextDisplayConfigs.end());
        indicators.assign(_cachedIndicatorConfigs.begin(),    _cachedIndicatorConfigs.end());
        colors.assign(_cachedColorConfigs.begin(),             _cachedColorConfigs.end());
        colorBindings.assign(_cachedColorBindingConfigs.begin(), _cachedColorBindingConfigs.end());
        _apiLock.Unlock();
    }

} // namespace Plugin
} // namespace WPEFramework
