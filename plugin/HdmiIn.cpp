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

#include "HdmiIn.h"
#include "DeviceSettingsTypes.h"

using IPlatform = hal::dHdmiIn::IPlatform;
using DefaultImpl = dHdmiInImpl;

#include "hal/dHdmiIn.h"
namespace hal {
namespace dHdmiIn {
    IPlatform::~IPlatform() {}
}
}

HdmiIn::HdmiIn(INotification& parent, std::shared_ptr<IPlatform> platform)
    : _platform(std::move(platform))
    , _parent(parent)
{
    Platform_init();
}

void HdmiIn::Platform_init()
{
    CallbackBundle bundle;
    bundle.OnHDMIInHotPlugEvent = [this](HDMIInPort port, bool isConnected) {
        this->OnHDMIInHotPlugEvent(port, isConnected);
    };
    bundle.OnHDMIInSignalStatusEvent = [this](HDMIInPort port, HDMIInSignalStatus signalStatus) {
        this->OnHDMIInSignalStatusEvent(port, signalStatus);
    };
    bundle.OnHDMIInStatusEvent = [this](HDMIInPort port, bool isConnected) {
        this->OnHDMIInStatusEvent(port, isConnected);
    };
    bundle.OnHDMIInVideoModeUpdateEvent = [this](HDMIInPort port, HDMIVideoPortResolution videoPortResolution) {
        this->OnHDMIInVideoModeUpdateEvent(port, videoPortResolution);
    };
    bundle.OnHDMIInAllmStatusEvent = [this](HDMIInPort port, bool allmStatus) {
        this->OnHDMIInAllmStatusEvent(port, allmStatus);
    };
    bundle.OnHDMIInAVIContentTypeEvent = [this](HDMIInPort port, HDMIInAviContentType aviContentType) {
        this->OnHDMIInAVIContentTypeEvent(port, aviContentType);
    };
    bundle.OnHDMIInAVLatencyEvent = [this](int32_t audioDelay, int32_t videoDelay) {
        this->OnHDMIInAVLatencyEvent(audioDelay, videoDelay);
    };
    bundle.OnHDMIInVRRStatusEvent = [this](HDMIInPort port, HDMIInVRRType vrrType) {
        this->OnHDMIInVRRStatusEvent(port, vrrType);
    };
    if (_platform) {
        this->platform().setAllCallbacks(bundle);
        this->platform().getPersistenceValue();
    }
}

void HdmiIn::OnHDMIInHotPlugEvent(const HDMIInPort port, const bool isConnected)
{
    _parent.OnHDMIInEventHotPlugNotification(port, isConnected);
}

void HdmiIn::OnHDMIInSignalStatusEvent(const HDMIInPort port, const HDMIInSignalStatus signalStatus)
{
    _parent.OnHDMIInEventSignalStatusNotification(port, signalStatus);
}

void HdmiIn::OnHDMIInStatusEvent(const HDMIInPort activePort, const bool isPresented)
{
    _parent.OnHDMIInEventStatusNotification(activePort, isPresented);
}

void HdmiIn::OnHDMIInVideoModeUpdateEvent(const HDMIInPort port, const HDMIVideoPortResolution videoPortResolution)
{
    _parent.OnHDMIInVideoModeUpdateNotification(port, videoPortResolution);
}

void HdmiIn::OnHDMIInAllmStatusEvent(const HDMIInPort port, const bool allmStatus)
{
    _parent.OnHDMIInAllmStatusNotification(port, allmStatus);
}

void HdmiIn::OnHDMIInAVIContentTypeEvent(const HDMIInPort port, const HDMIInAviContentType aviContentType)
{
    _parent.OnHDMIInAVIContentTypeNotification(port, aviContentType);
}

void HdmiIn::OnHDMIInAVLatencyEvent(const int32_t audioDelay, const int32_t videoDelay)
{
    _parent.OnHDMIInAVLatencyNotification(audioDelay, videoDelay);
}

void HdmiIn::OnHDMIInVRRStatusEvent(const HDMIInPort port, const HDMIInVRRType vrrType)
{
    _parent.OnHDMIInVRRStatusNotification(port, vrrType);
}

uint32_t HdmiIn::GetHDMIInNumberOfInputs(int32_t &count) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    result = this->platform().GetHDMIInNumberOfInputs(count);
    DSLOG_INFO("result=%u, count=%d", result, count);

    return result;
}

uint32_t HdmiIn::GetHDMIInStatus(HDMIInStatus &hdmiStatus, IHDMIInPortConnectionStatusIterator*& portConnectionStatus) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;

    result = this->platform().GetHDMIInStatus(hdmiStatus, portConnectionStatus);
    DSLOG_INFO("result=%u", result);

    return result;
}

uint32_t HdmiIn::SelectHDMIInPort(const HDMIInPort port, const bool requestAudioMix, const bool topMostPlane, const HDMIVideoPlaneType videoPlaneType) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    DSLOG_INFO("port=%d, requestAudioMix=%s, topMostPlane=%s, videoPlaneType=%d",
        port, requestAudioMix ? "true" : "false", topMostPlane ? "true" : "false", videoPlaneType);
    result = this->platform().SelectHDMIInPort(port, requestAudioMix, topMostPlane, videoPlaneType);
    DSLOG_INFO("result=%u", result);

    return result;
}

uint32_t HdmiIn::ScaleHDMIInVideo(const HDMIInVideoRectangle videoPosition) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    DSLOG_INFO("x=%d, y=%d, w=%d, h=%d", videoPosition.x, videoPosition.y, videoPosition.width, videoPosition.height);
    result = this->platform().ScaleHDMIInVideo(videoPosition);
    DSLOG_INFO("result=%u", result);

    return result;
}

uint32_t HdmiIn::SelectHDMIZoomMode(const HDMIInVideoZoom zoomMode) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    DSLOG_INFO("zoomMode=%d", zoomMode);
    result = this->platform().SelectHDMIZoomMode(zoomMode);
    DSLOG_INFO("result=%u", result);

    return result;
}

uint32_t HdmiIn::GetSupportedGameFeaturesList(IHDMIInGameFeatureListIterator *& gameFeatureList) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    result = this->platform().GetSupportedGameFeaturesList(gameFeatureList);
    DSLOG_INFO("result=%u", result);
    return result;
}

uint32_t HdmiIn::GetHDMIInAVLatency(uint32_t &videoLatency, uint32_t &audioLatency) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    result = this->platform().GetHDMIInAVLatency(videoLatency, audioLatency);
    DSLOG_INFO("result=%u, videoLatency=%u, audioLatency=%u", result, videoLatency, audioLatency);

    return result;
}

uint32_t HdmiIn::GetHDMIInAllmStatus(const HDMIInPort port, bool &allmStatus) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    DSLOG_INFO("port=%d", port);
    result = this->platform().GetHDMIInAllmStatus(port, allmStatus);
    DSLOG_INFO("result=%u, port=%d, allmStatus=%s", result, port, allmStatus ? "true" : "false");
    return result;
}

uint32_t HdmiIn::GetHDMIInEdid2AllmSupport(const HDMIInPort port, bool &allmSupport) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    DSLOG_INFO("port=%d", port);
    result = this->platform().GetHDMIInEdid2AllmSupport(port, allmSupport);
    DSLOG_INFO("result=%u, port=%d, allmSupport=%s", result, port, allmSupport ? "true" : "false");
    return result;
}

uint32_t HdmiIn::SetHDMIInEdid2AllmSupport(const HDMIInPort port, bool allmSupport) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    DSLOG_INFO("port=%d, allmSupport=%s", port, allmSupport ? "true" : "false");
    result = this->platform().SetHDMIInEdid2AllmSupport(port, allmSupport);
    DSLOG_INFO("result=%u", result);
    return result;
}

uint32_t HdmiIn::GetEdidBytes(const HDMIInPort port, const uint16_t edidBytesLength, uint8_t edidBytes[]) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    DSLOG_INFO("port=%d, edidBytesLength=%u", port, edidBytesLength);
    result = this->platform().GetEdidBytes(port, edidBytesLength, edidBytes);
    DSLOG_INFO("result=%u", result);

    return result;
}

uint32_t HdmiIn::GetHDMISPDInformation(const HDMIInPort port, const uint16_t spdBytesLength, uint8_t spdBytes[]) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    DSLOG_INFO("port=%d, spdBytesLength=%u", port, spdBytesLength);
    result = this->platform().GetHDMISPDInformation(port, spdBytesLength, spdBytes);
    DSLOG_INFO("result=%u", result);

    return result;
}

uint32_t HdmiIn::GetHDMIEdidVersion(const HDMIInPort port, HDMIInEdidVersion &edidVersion) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    DSLOG_INFO("port=%d", port);
    result = this->platform().GetHDMIEdidVersion(port, edidVersion);
    DSLOG_INFO("result=%u, port=%d, edidVersion=%d", result, port, edidVersion);
    return result;
}

uint32_t HdmiIn::SetHDMIEdidVersion(const HDMIInPort port, const HDMIInEdidVersion edidVersion) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    result = this->platform().SetHDMIEdidVersion(port, edidVersion);
    DSLOG_INFO("result=%u, port=%d, edidVersion=%d", result, port, edidVersion);

    return result;
}

uint32_t HdmiIn::GetHDMIVideoMode(HDMIVideoPortResolution &videoPortResolution) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;

    result = this->platform().GetHDMIVideoMode(videoPortResolution);
    DSLOG_INFO("result=%u", result);

    return result;
}

uint32_t HdmiIn::GetHDMIVersion(const HDMIInPort port, HDMIInCapabilityVersion &capabilityVersion) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;

    DSLOG_INFO("port=%d", port);
    result = this->platform().GetHDMIVersion(port, capabilityVersion);
    DSLOG_INFO("result=%u, port=%d, capabilityVersion=%d", result, port, capabilityVersion);
    return result;
}

uint32_t HdmiIn::GetVRRSupport(const HDMIInPort port, bool &vrrSupport) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;

    DSLOG_INFO("port=%d", port);
    result = this->platform().GetVRRSupport(port, vrrSupport);
    DSLOG_INFO("result=%u, port=%d, vrrSupport=%s", result, port, vrrSupport ? "true" : "false");
    return result;
}

uint32_t HdmiIn::SetVRRSupport(const HDMIInPort port, const bool vrrSupport) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;

    DSLOG_INFO("port=%d, vrrSupport=%s", port, vrrSupport ? "true" : "false");
    result = this->platform().SetVRRSupport(port, vrrSupport);
    DSLOG_INFO("result=%u", result);
    return result;
}

uint32_t HdmiIn::GetVRRStatus(const HDMIInPort port, HDMIInVRRStatus &vrrStatus) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;

    DSLOG_INFO("port=%d", port);
    memset(&vrrStatus, 0, sizeof(vrrStatus));
    result = this->platform().GetVRRStatus(port, vrrStatus);
    DSLOG_INFO("result=%u, port=%d, vrrType=%d", result, port, vrrStatus.vrrType);

    return result;
}