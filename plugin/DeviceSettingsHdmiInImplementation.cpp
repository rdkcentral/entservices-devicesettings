/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

#include "DeviceSettingsHdmiInImplementation.h"

#include <syscall.h>

using namespace std;

namespace WPEFramework {
namespace Plugin {

    DeviceSettingsHdmiInImp::DeviceSettingsHdmiInImp()
        : _hdmiIn(HdmiIn::Create(*this))
    {
        DSLOG_INFO("Constructor - Instance Address: %p", this);
    }

    DeviceSettingsHdmiInImp::~DeviceSettingsHdmiInImp() {
        DSLOG_INFO("Destructor - Instance Address: %p", this);
    }

    template<typename Func, typename... Args>
    void DeviceSettingsHdmiInImp::dispatchHDMIInEvent(Func notifyFunc, Args&&... args) {
        DSLOG_INFO(">>");
        _callbackLock.Lock();
        for (auto& entry : _HDMIInNotifications) {
            const string& clientName = entry.first;
            auto* notification = entry.second;
            auto start = std::chrono::steady_clock::now();
            (notification->*notifyFunc)(std::forward<Args>(args)...);
            auto elapsed = std::chrono::steady_clock::now() - start;
            DSLOG_INFO("client '%s' took %" PRId64 "ms to process IHDMIIn event", clientName.c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
        }
        _callbackLock.Unlock();
        DSLOG_INFO("<<");
    }

    template <typename T>
    Core::hresult DeviceSettingsHdmiInImp::Register(std::list<std::pair<string, T*>>& list, const string& clientName, T* notification)
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
    Core::hresult DeviceSettingsHdmiInImp::Unregister(std::list<std::pair<string, T*>>& list, const T* notification)
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


    Core::hresult DeviceSettingsHdmiInImp::Register(const string& clientName, DeviceSettingsHDMIIn::INotification* notification)
    {
        Core::hresult errorCode = Register(_HDMIInNotifications, clientName, notification);
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_ERR("IHDMIIn %p [%s], errorCode: %u", notification, clientName.c_str(), errorCode);
        } else {
            DSLOG_INFO("IHDMIIn %p [%s] registered successfully", notification, clientName.c_str());
        }
        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::Unregister(DeviceSettingsHDMIIn::INotification* notification)
    {
        Core::hresult errorCode = Unregister(_HDMIInNotifications, notification);
        if (errorCode != Core::ERROR_NONE) {
            DSLOG_ERR("IHDMIIn %p, errorcode: %u", notification, errorCode);
        } else {
            DSLOG_INFO("IHDMIIn %p unregistered successfully", notification);
        }
        return errorCode;
    }

    void DeviceSettingsHdmiInImp::OnHDMIInEventHotPlugNotification(const HDMIInPort port, const bool isConnected)
    {
        DSLOG_INFO("OnHDMIInEventHotPlug event Received");
        dispatchHDMIInEvent(&DeviceSettingsHDMIIn::INotification::OnHDMIInEventHotPlug, port, isConnected);
    }

    void DeviceSettingsHdmiInImp::OnHDMIInEventSignalStatusNotification(const HDMIInPort port, const HDMIInSignalStatus signalStatus)
    {
        DSLOG_INFO("OnHDMIInEventSignalStatus event Received");
        dispatchHDMIInEvent(&DeviceSettingsHDMIIn::INotification::OnHDMIInEventSignalStatus, port, signalStatus);
    }

    void DeviceSettingsHdmiInImp::OnHDMIInAVLatencyNotification(const int32_t audioDelay, const int32_t videoDelay)
    {
        DSLOG_INFO("OnHDMIInAVLatency event Received");
        dispatchHDMIInEvent(&DeviceSettingsHDMIIn::INotification::OnHDMIInAVLatency, audioDelay, videoDelay);
    }

    void DeviceSettingsHdmiInImp::OnHDMIInEventStatusNotification(const HDMIInPort activePort, const bool isPresented)
    {
        DSLOG_INFO("OnHDMIInEventStatus event Received");
        dispatchHDMIInEvent(&DeviceSettingsHDMIIn::INotification::OnHDMIInEventStatus, activePort, isPresented);
    }

    void DeviceSettingsHdmiInImp::OnHDMIInVideoModeUpdateNotification(const HDMIInPort port, const HDMIVideoPortResolution videoPortResolution)
    {
        DSLOG_INFO("OnHDMIInVideoModeUpdate event Received");
        dispatchHDMIInEvent(&DeviceSettingsHDMIIn::INotification::OnHDMIInVideoModeUpdate, port, videoPortResolution);
    }

    void DeviceSettingsHdmiInImp::OnHDMIInAllmStatusNotification(const HDMIInPort port, const bool allmStatus)
    {
        DSLOG_INFO("OnHDMIInAllmStatus event Received");
        dispatchHDMIInEvent(&DeviceSettingsHDMIIn::INotification::OnHDMIInAllmStatus, port, allmStatus);
    }

    void DeviceSettingsHdmiInImp::OnHDMIInAVIContentTypeNotification(const HDMIInPort port, const HDMIInAviContentType aviContentType)
    {
        DSLOG_INFO("OnHDMIInAVIContentType event Received");
        dispatchHDMIInEvent(&DeviceSettingsHDMIIn::INotification::OnHDMIInAVIContentType, port, aviContentType);
    }

    void DeviceSettingsHdmiInImp::OnHDMIInVRRStatusNotification(const HDMIInPort port, const HDMIInVRRType vrrType)
    {
        DSLOG_INFO("OnHDMIInVRRStatus event Received");
        dispatchHDMIInEvent(&DeviceSettingsHDMIIn::INotification::OnHDMIInVRRStatus, port, vrrType);
    }

    Core::hresult DeviceSettingsHdmiInImp::GetHDMIInNumberOfInputs(int32_t &count) {

        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.GetHDMIInNumberOfInputs(count) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - count=%d", count);

        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::GetHDMIInStatus(HDMIInStatus &hdmiStatus, IHDMIInPortConnectionStatusIterator*& portConnectionStatus) {

        Core::hresult errorCode = Core::ERROR_GENERAL;

        _apiLock.Lock();
        if (_hdmiIn.GetHDMIInStatus(hdmiStatus, portConnectionStatus) == dsERR_NONE) {
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
        DSLOG_INFO("activePort=%d, isPresented=%s", hdmiStatus.activePort, hdmiStatus.isPresented ? "true" : "false");


        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::SelectHDMIInPort(const HDMIInPort port, const bool requestAudioMix, const bool topMostPlane, const HDMIVideoPlaneType videoPlaneType) {

        DSLOG_INFO("port=%d, requestAudioMix=%s, topMostPlane=%s, videoPlaneType=%d",
            port, requestAudioMix ? "true" : "false", topMostPlane ? "true" : "false", videoPlaneType);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.SelectHDMIInPort(port, requestAudioMix, topMostPlane, videoPlaneType) == dsERR_NONE) {
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

    Core::hresult DeviceSettingsHdmiInImp::ScaleHDMIInVideo(const HDMIInVideoRectangle videoPosition) {

        DSLOG_INFO("x=%d, y=%d, w=%d, h=%d", videoPosition.x, videoPosition.y, videoPosition.width, videoPosition.height);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.ScaleHDMIInVideo(videoPosition) == dsERR_NONE) {
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

    Core::hresult DeviceSettingsHdmiInImp::SelectHDMIZoomMode(const HDMIInVideoZoom zoomMode) {

        DSLOG_INFO("zoomMode=%d", zoomMode);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.SelectHDMIZoomMode(zoomMode) == dsERR_NONE) {
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

    Core::hresult DeviceSettingsHdmiInImp::GetSupportedGameFeaturesList(IHDMIInGameFeatureListIterator *& gameFeatureList) {

        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.GetSupportedGameFeaturesList(gameFeatureList) == dsERR_NONE) {
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

    Core::hresult DeviceSettingsHdmiInImp::GetHDMIInAVLatency(uint32_t &videoLatency, uint32_t &audioLatency) {

        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.GetHDMIInAVLatency(videoLatency, audioLatency) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - videoLatency=%u, audioLatency=%u", videoLatency, audioLatency);

        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::GetHDMIInAllmStatus(const HDMIInPort port, bool &allmStatus) {

        DSLOG_INFO("port=%d", port);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.GetHDMIInAllmStatus(port, allmStatus) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }
        
        DSLOG_INFO("SUCCESS - port=%d, allmStatus=%s", port, allmStatus ? "true" : "false");

        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::GetHDMIInEdid2AllmSupport(const HDMIInPort port, bool &allmSupport) {

        DSLOG_INFO("port=%d", port);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.GetHDMIInEdid2AllmSupport(port, allmSupport) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }
        
        DSLOG_INFO("SUCCESS - port=%d, allmSupport=%s", port, allmSupport ? "true" : "false");

        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::SetHDMIInEdid2AllmSupport(const HDMIInPort port, bool allmSupport) {

        DSLOG_INFO("port=%d, allmSupport=%s", port, allmSupport ? "true" : "false");
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.SetHDMIInEdid2AllmSupport(port, allmSupport) == dsERR_NONE) {
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

    Core::hresult DeviceSettingsHdmiInImp::GetEdidBytes(const HDMIInPort port, const uint16_t edidBytesLength, uint8_t edidBytes[]) {

        DSLOG_INFO("port=%d, edidBytesLength=%u", port, edidBytesLength);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.GetEdidBytes(port, edidBytesLength, edidBytes) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - port=%d, edidBytes[0]=0x%X", port, edidBytes[0]);


        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::GetHDMISPDInformation(const HDMIInPort port, const uint16_t spdBytesLength, uint8_t spdBytes[]) {

        DSLOG_INFO("port=%d, spdBytesLength=%u", port, spdBytesLength);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        if (spdBytes && spdBytesLength > 0) {
            spdBytes[0] = 0x00; // Example value
        }
        _apiLock.Lock();
        if (_hdmiIn.GetHDMISPDInformation(port, spdBytesLength, spdBytes) == dsERR_NONE) {
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
        DSLOG_INFO("port=%d, spdBytes[0]=0x%X", port, spdBytes[0]);


        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::GetHDMIEdidVersion(const HDMIInPort port, HDMIInEdidVersion &edidVersion) {

        DSLOG_INFO("port=%d", port);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.GetHDMIEdidVersion(port, edidVersion) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - port=%d, edidVersion=%d", port, edidVersion);

        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::SetHDMIEdidVersion(const HDMIInPort port, const HDMIInEdidVersion edidVersion) {

        DSLOG_INFO("port=%d, edidVersion=%d", port, edidVersion);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.SetHDMIEdidVersion(port, edidVersion) == dsERR_NONE) {
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

    Core::hresult DeviceSettingsHdmiInImp::GetHDMIVideoMode(HDMIVideoPortResolution &videoPortResolution) {

        Core::hresult errorCode = Core::ERROR_GENERAL;

        _apiLock.Lock();
        if (_hdmiIn.GetHDMIVideoMode(videoPortResolution) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - resolution=%s", videoPortResolution.name.c_str());

        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::GetHDMIVersion(const HDMIInPort port, HDMIInCapabilityVersion &capabilityVersion) {

        DSLOG_INFO("port=%d", port);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.GetHDMIVersion(port, capabilityVersion) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - port=%d, capabilityVersion=%d", port, capabilityVersion);

        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::GetVRRSupport(const HDMIInPort port, bool &vrrSupport) {

        DSLOG_INFO("port=%d", port);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.GetVRRSupport(port, vrrSupport) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - port=%d, vrrSupport=%s", port, vrrSupport ? "true" : "false");

        return errorCode;
    }

    Core::hresult DeviceSettingsHdmiInImp::SetVRRSupport(const HDMIInPort port, const bool vrrSupport) {

        DSLOG_INFO("port=%d, vrrSupport=%s", port, vrrSupport ? "true" : "false");
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.SetVRRSupport(port, vrrSupport) == dsERR_NONE) {
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

    Core::hresult DeviceSettingsHdmiInImp::GetVRRStatus(const HDMIInPort port, HDMIInVRRStatus &vrrStatus) {

        DSLOG_INFO("port=%d", port);
        Core::hresult errorCode = Core::ERROR_GENERAL;
        _apiLock.Lock();
        if (_hdmiIn.GetVRRStatus(port, vrrStatus) == dsERR_NONE) {
            errorCode = Core::ERROR_NONE;
        } else {
            errorCode = Core::ERROR_GENERAL;
        }
        _apiLock.Unlock();

        if (errorCode != Core::ERROR_NONE) {
            DSLOG_WARN("failed with errorCode=%u", errorCode);
            return errorCode;
        }

        DSLOG_INFO("SUCCESS - port=%d, vrrStatus.vrrType=%d", port, vrrStatus.vrrType);

        return errorCode;
    }

} // namespace Plugin
} // namespace WPEFramework
