/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include "Module.h"

#include <interfaces/IDeviceSettingsAudio.h>
#include <interfaces/IDeviceSettingsCompositeIn.h>
#include <interfaces/IDeviceSettingsDisplay.h>
#include <interfaces/IDeviceSettingsFPD.h>
#include <interfaces/IDeviceSettingsHDMIIn.h>
#include <interfaces/IDeviceSettingsHost.h>
#include <interfaces/IDeviceSettingsVideoDevice.h>
#include <interfaces/IDeviceSettingsVideoPort.h>

#include <com/com.h>
#include <core/core.h>
#include <plugins/plugins.h>

#include "DeviceSettingsTypes.h"


namespace WPEFramework {
namespace Plugin {

    class DeviceSettings : public PluginHost::IPlugin
    {
    private:
        class NotificationHandler : public RPC::IRemoteConnection::INotification
                                  , public PluginHost::IShell::ICOMLink::INotification
                                  , public DeviceSettingsCompositeIn::INotification
                                  , public DeviceSettingsAudio::INotification
                                  , public DeviceSettingsFPD::INotification
                                  , public DeviceSettingsDisplay::INotification
                                  , public DeviceSettingsHDMIIn::INotification
                                  , public DeviceSettingsVideoPort::INotification
                                  , public DeviceSettingsVideoDevice::INotification
            {
        private:
            NotificationHandler()                                      = delete;
            NotificationHandler(const NotificationHandler&)            = delete;
            NotificationHandler& operator=(const NotificationHandler&) = delete;

        public:
            explicit NotificationHandler(DeviceSettings* parent)
                : mParent(*parent)
            {
                ASSERT(parent != nullptr);
            }

            virtual ~NotificationHandler()
            {
            }

            template <typename T>
            T* baseInterface()
            {
                static_assert(std::is_base_of<T, NotificationHandler>(), "base type mismatch");
                return static_cast<T*>(this);
            }

            BEGIN_INTERFACE_MAP(NotificationHandler)
            INTERFACE_ENTRY(DeviceSettingsCompositeIn::INotification)
            INTERFACE_ENTRY(DeviceSettingsAudio::INotification)
            INTERFACE_ENTRY(DeviceSettingsFPD::INotification)
            INTERFACE_ENTRY(DeviceSettingsDisplay::INotification)
            INTERFACE_ENTRY(DeviceSettingsHDMIIn::INotification)
            INTERFACE_ENTRY(DeviceSettingsVideoPort::INotification)
            INTERFACE_ENTRY(DeviceSettingsVideoDevice::INotification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

            void Activated(RPC::IRemoteConnection*) override
            {
            }

            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                mParent.Deactivated(connection);
            }

            void Dangling(const Core::IUnknown* remote, const uint32_t interfaceId) override
            {
                ASSERT(remote != nullptr);
                mParent.CallbackRevoked(remote, interfaceId);
            }

            void Revoked(const Core::IUnknown* remote, const uint32_t interfaceId) override
            {
                ASSERT(remote != nullptr);
                mParent.CallbackRevoked(remote, interfaceId);
            }

            void OnFPDTimeFormatChanged(const FPDTimeFormat timeFormat) override
            {
                DSLOG_INFO("timeFormat %d", timeFormat);
            }

            // Audio notification handlers
            void OnAssociatedAudioMixingChanged(bool mixing) override
            {
                DSLOG_INFO("mixing %d", mixing);
            }

            void OnAudioFaderControlChanged(int32_t mixerBalance) override
            {
                DSLOG_INFO("mixerBalance %d", mixerBalance);
            }

            void OnAudioPrimaryLanguageChanged(const string& primaryLanguage) override
            {
                DSLOG_INFO("primaryLanguage %s", primaryLanguage.c_str());
            }

            void OnAudioSecondaryLanguageChanged(const string& secondaryLanguage) override
            {
                DSLOG_INFO("secondaryLanguage %s", secondaryLanguage.c_str());
            }

            void OnAudioOutHotPlug(AudioPortType portType, uint32_t uiPortNumber, bool isPortConnected) override
            {
                DSLOG_INFO("portType %d, port %d, connected %d", portType, uiPortNumber, isPortConnected);
            }

            void OnAudioFormatUpdate(AudioFormat audioFormat) override
            {
                DSLOG_INFO("audioFormat %d", audioFormat);
            }

            void OnDolbyAtmosCapabilitiesChanged(DolbyAtmosCapability atmosCapability, bool status) override
            {
                DSLOG_INFO("capability %d, status %d", atmosCapability, status);
            }

            void OnAudioPortStateChanged(AudioPortState audioPortState) override
            {
                DSLOG_INFO("state %d", audioPortState);
            }

            void OnAudioLevelChanged(int32_t audioLevel) override
            {
                DSLOG_INFO("level %d", audioLevel);
            }

            void OnAudioModeEvent(AudioPortType audioPortType, AudioStereoMode audioMode) override
            {
                DSLOG_INFO("portType %d, mode %d", audioPortType, audioMode);
            }

            void OnHDMIInEventHotPlug(const HDMIInPort port, const bool isConnected) override 
            {
                DSLOG_INFO("port=%d, connected=%s", static_cast<int>(port), isConnected ? "true" : "false");
            }

            void OnHDMIInEventSignalStatus(const HDMIInPort port, const HDMIInSignalStatus signalStatus) override
            {
                DSLOG_INFO("port=%d, signalStatus=%d", static_cast<int>(port), static_cast<int>(signalStatus));
            }

            void OnHDMIInEventStatus(const HDMIInPort activePort, const bool isPresented) override
            {
                DSLOG_INFO("activePort=%d, presented=%s", static_cast<int>(activePort), isPresented ? "true" : "false");
            }

            void OnHDMIInVideoModeUpdate(const HDMIInPort port, const HDMIVideoPortResolution& videoPortResolution) override
            {
                DSLOG_INFO("port=%d, name=%s, pixelResolution=%d, aspectRatio=%d, stereoScopicMode=%d, frameRate=%d, interlaced=%s",
                    static_cast<int>(port), videoPortResolution.name.c_str(), static_cast<int>(videoPortResolution.pixelResolution),
                    static_cast<int>(videoPortResolution.aspectRatio), static_cast<int>(videoPortResolution.stereoScopicMode),
                    static_cast<int>(videoPortResolution.frameRate), videoPortResolution.interlaced ? "true" : "false");
            }

            void OnHDMIInAllmStatus(const HDMIInPort port, const bool allmStatus) override
            {
                DSLOG_INFO("port=%d, allmStatus=%s", static_cast<int>(port), allmStatus ? "true" : "false");
            }

            void OnHDMIInAVIContentType(const HDMIInPort port, const HDMIInAviContentType aviContentType) override
            {
                DSLOG_INFO("port=%d, aviContentType=%d", static_cast<int>(port), static_cast<int>(aviContentType));
            }

            void OnHDMIInAVLatency(const int32_t audioDelay, const int32_t videoDelay) override
            {
                DSLOG_INFO("audioDelay=%d, videoDelay=%d", audioDelay, videoDelay);
            }

            void OnHDMIInVRRStatus(const HDMIInPort port, const HDMIInVRRType vrrType) override
            {
                DSLOG_INFO("port=%d, vrrType=%d", static_cast<int>(port), static_cast<int>(vrrType));
            }

            // VideoPort notification handlers matching WPE interface
            void OnResolutionPostChange(const ResolutionChange& resolution) override
            {
                DSLOG_INFO("width=%u, height=%u", resolution.width, resolution.height);
            }

            void OnResolutionPreChange(const ResolutionChange& resolution) override
            {
                DSLOG_INFO("width=%u, height=%u", resolution.width, resolution.height);
            }

            void OnHDCPStatusChange(const Exchange::IDeviceSettingsVideoPort::HDCPStatus hdcpStatus) override
            {
                DSLOG_INFO("status=%d", (int)hdcpStatus);
            }

            void OnVideoFormatUpdate(const Exchange::IDeviceSettingsVideoPort::HDRStandard videoFormatHDR) override
            {
                DSLOG_INFO("hdrStandard=%d", (int)videoFormatHDR);
            }

            // CompositeIn notification handlers matching WPE interface
            void OnCompositeInHotPlug(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort port, const bool isConnected) override
            {
                DSLOG_INFO("port=%d, isConnected=%s", (int)port, isConnected ? "true" : "false");
            }

            void OnCompositeInSignalStatus(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort port, const Exchange::IDeviceSettingsCompositeIn::CompositeInSignalStatus signalStatus) override
            {
                DSLOG_INFO("port=%d, signalStatus=%d", (int)port, (int)signalStatus);
            }

            void OnCompositeInStatus(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort activePort, const bool isPresented) override
            {
                DSLOG_INFO("activePort=%d, isPresented=%s", (int)activePort, isPresented ? "true" : "false");
            }

            void OnCompositeInVideoModeUpdate(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort activePort, const Exchange::IDeviceSettingsCompositeIn::DisplayVideoPortResolution& videoResolution) override
            {
                DSLOG_INFO("activePort=%d, resolution=%s", (int)activePort, videoResolution.name.c_str());
            }

            // VideoDevice event handlers (matching actual IDeviceSettingsVideoDevice::INotification interface)
            void OnZoomSettingsChanged(const Exchange::IDeviceSettingsVideoDevice::VideoZoom zoomSetting) override
            {
                DSLOG_INFO("zoomSetting=%d", static_cast<int>(zoomSetting));
            }

            void OnDisplayFrameratePreChange(const string& frameRate) override
            {
                DSLOG_INFO("frameRate=%s", frameRate.c_str());
            }

            void OnDisplayFrameratePostChange(const string& frameRate) override
            {
                DSLOG_INFO("frameRate=%s", frameRate.c_str());
            }

        private:
            DeviceSettings& mParent;
        };
    public:
        DeviceSettings(const DeviceSettings&) = delete;
        DeviceSettings(DeviceSettings&&) = delete;
        DeviceSettings& operator=(const DeviceSettings&) = delete;
        DeviceSettings& operator=(DeviceSettings&) = delete;

        DeviceSettings();
        virtual ~DeviceSettings();

        // Build QueryInterface implementation, specifying all possible interfaces to be returned.
        BEGIN_INTERFACE_MAP(DeviceSettings)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_AGGREGATE(Exchange::IDeviceSettings, _mDeviceSettings)
            INTERFACE_AGGREGATE(Exchange::IDeviceSettingsCompositeIn, _mDeviceSettingsCompositeIn)
            INTERFACE_AGGREGATE(Exchange::IDeviceSettingsAudio, _mDeviceSettingsAudio)
            INTERFACE_AGGREGATE(Exchange::IDeviceSettingsFPD, _mDeviceSettingsFPD)
            INTERFACE_AGGREGATE(Exchange::IDeviceSettingsDisplay, _mDeviceSettingsDisplay)
            INTERFACE_AGGREGATE(Exchange::IDeviceSettingsHDMIIn, _mDeviceSettingsHDMIIn)
            INTERFACE_AGGREGATE(Exchange::IDeviceSettingsHost, _mDeviceSettingsHost)
            INTERFACE_AGGREGATE(Exchange::IDeviceSettingsVideoPort, _mDeviceSettingsVideoPort)
            INTERFACE_AGGREGATE(Exchange::IDeviceSettingsVideoDevice, _mDeviceSettingsVideoDevice)
        END_INTERFACE_MAP

    public:

        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

    private:
        void Deactivated(RPC::IRemoteConnection* connection);
        void CallbackRevoked(const Core::IUnknown* remote, const uint32_t interfaceId);

    private:
        uint32_t mConnectionId;
        PluginHost::IShell* mService;
        Exchange::IDeviceSettings* _mDeviceSettings;
        Exchange::IDeviceSettingsCompositeIn* _mDeviceSettingsCompositeIn;
        DeviceSettingsAudio* _mDeviceSettingsAudio;
        Exchange::IDeviceSettingsFPD* _mDeviceSettingsFPD;
        Exchange::IDeviceSettingsDisplay* _mDeviceSettingsDisplay;
        Exchange::IDeviceSettingsHDMIIn* _mDeviceSettingsHDMIIn;
        Exchange::IDeviceSettingsHost* _mDeviceSettingsHost;
        Exchange::IDeviceSettingsVideoPort* _mDeviceSettingsVideoPort;
        Exchange::IDeviceSettingsVideoDevice* _mDeviceSettingsVideoDevice;
        Core::Sink<NotificationHandler> mNotificationSink;

    };

} // namespace Plugin
} // namespace WPEFramework
