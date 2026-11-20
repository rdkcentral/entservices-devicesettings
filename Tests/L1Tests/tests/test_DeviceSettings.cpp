/*
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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

#include <gtest/gtest.h>
#include <vector>

#include "DeviceSettingsImplementation.h"
#include "Module.h"
#include "DeviceSettingsHALConfig.h"
#include "hal/dHdmiInImpl.h"

using namespace WPEFramework;

namespace {

TEST(DeviceSettingsImpTest, ExposesMainInterface)
{
    Core::ProxyType<Plugin::DeviceSettingsImp> implementation = Core::ProxyType<Plugin::DeviceSettingsImp>::Create();

    Exchange::IDeviceSettings* deviceSettings = static_cast<Exchange::IDeviceSettings*>(
        implementation->QueryInterface(Exchange::IDeviceSettings::ID));
    ASSERT_NE(nullptr, deviceSettings);

    deviceSettings->Release();
}

TEST(DeviceSettingsImpTest, ExposesComponentInterfaces)
{
    Core::ProxyType<Plugin::DeviceSettingsImp> implementation = Core::ProxyType<Plugin::DeviceSettingsImp>::Create();

    Exchange::IDeviceSettingsFPD* fpd = static_cast<Exchange::IDeviceSettingsFPD*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsFPD::ID));
    Exchange::IDeviceSettingsHDMIIn* hdmiIn = static_cast<Exchange::IDeviceSettingsHDMIIn*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsHDMIIn::ID));
    Exchange::IDeviceSettingsAudio* audio = static_cast<Exchange::IDeviceSettingsAudio*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsAudio::ID));
    Exchange::IDeviceSettingsVideoPort* videoPort = static_cast<Exchange::IDeviceSettingsVideoPort*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsVideoPort::ID));
    Exchange::IDeviceSettingsVideoDevice* videoDevice = static_cast<Exchange::IDeviceSettingsVideoDevice*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsVideoDevice::ID));
    Exchange::IDeviceSettingsHost* host = static_cast<Exchange::IDeviceSettingsHost*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsHost::ID));
    Exchange::IDeviceSettingsCompositeIn* compositeIn = static_cast<Exchange::IDeviceSettingsCompositeIn*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsCompositeIn::ID));
    Exchange::IDeviceSettingsDisplay* display = static_cast<Exchange::IDeviceSettingsDisplay*>(
        implementation->QueryInterface(Exchange::IDeviceSettingsDisplay::ID));

    EXPECT_NE(nullptr, fpd);
    EXPECT_NE(nullptr, hdmiIn);
    EXPECT_NE(nullptr, audio);
    EXPECT_NE(nullptr, videoPort);
    EXPECT_NE(nullptr, videoDevice);
    EXPECT_NE(nullptr, host);
    EXPECT_NE(nullptr, compositeIn);
    EXPECT_NE(nullptr, display);

    if (fpd != nullptr) {
        fpd->Release();
    }
    if (hdmiIn != nullptr) {
        hdmiIn->Release();
    }
    if (audio != nullptr) {
        audio->Release();
    }
    if (videoPort != nullptr) {
        videoPort->Release();
    }
    if (videoDevice != nullptr) {
        videoDevice->Release();
    }
    if (host != nullptr) {
        host->Release();
    }
    if (compositeIn != nullptr) {
        compositeIn->Release();
    }
    if (display != nullptr) {
        display->Release();
    }
}

TEST(HdmiInPortBoundsTest, RejectsInvalidCacheReadsWithoutChangingOutputs)
{
    dHdmiInImpl platform;
    for (const int invalid : { -128, -1, static_cast<int>(dsHDMI_IN_PORT_MAX),
             static_cast<int>(dsHDMI_IN_PORT_MAX) + 1 }) {
        const auto port = static_cast<HDMIInPort>(invalid);
        for (const bool initial : { false, true }) {
            bool allm = initial;
            bool vrr = initial;
            EXPECT_EQ(Core::ERROR_GENERAL, platform.GetHDMIInEdid2AllmSupport(port, allm));
            EXPECT_EQ(Core::ERROR_GENERAL, platform.GetVRRSupport(port, vrr));
            EXPECT_EQ(initial, allm);
            EXPECT_EQ(initial, vrr);
        }
    }
}

TEST(HdmiInPortBoundsTest, RejectsInvalidCacheWritesAndEdidVersionUpdates)
{
    dHdmiInImpl platform;
    for (const int invalid : { -128, -1, static_cast<int>(dsHDMI_IN_PORT_MAX),
             static_cast<int>(dsHDMI_IN_PORT_MAX) + 1 }) {
        const auto port = static_cast<HDMIInPort>(invalid);
        EXPECT_EQ(Core::ERROR_GENERAL, platform.SetHDMIInEdid2AllmSupport(port, true));
        EXPECT_EQ(Core::ERROR_GENERAL, platform.SetVRRSupport(port, true));
        EXPECT_EQ(Core::ERROR_GENERAL, platform.SetHDMIEdidVersion(
            port, static_cast<HDMIInEdidVersion>(HDMI_EDID_VER_20)));
        EXPECT_EQ(dsERR_INVALID_PARAM, dHdmiInImpl::setVRRSupport(
            static_cast<dsHdmiInPort_t>(invalid), true));
    }
}

TEST(HdmiInPortBoundsTest, AcceptsValidCacheReadBoundaries)
{
    dHdmiInImpl platform;
    for (const int valid : { static_cast<int>(dsHDMI_IN_PORT_0),
             static_cast<int>(dsHDMI_IN_PORT_MAX) - 1 }) {
        const auto port = static_cast<HDMIInPort>(valid);
        bool allm = false;
        bool vrr = false;
        ASSERT_EQ(Core::ERROR_NONE, platform.GetHDMIInEdid2AllmSupport(port, allm));
        ASSERT_EQ(Core::ERROR_NONE, platform.GetVRRSupport(port, vrr));
        bool repeatedAllm = !allm;
        bool repeatedVrr = !vrr;
        ASSERT_EQ(Core::ERROR_NONE, platform.GetHDMIInEdid2AllmSupport(port, repeatedAllm));
        ASSERT_EQ(Core::ERROR_NONE, platform.GetVRRSupport(port, repeatedVrr));
        EXPECT_EQ(allm, repeatedAllm);
        EXPECT_EQ(vrr, repeatedVrr);
    }
}

struct VideoPortHandleTestPort {
    int videoPortType;
    int videoPortIndex;
    intptr_t nativeHandle;
    bool available;
};

bool GetVideoPortHandleForTest(const VideoPortHandleTestPort& port, intptr_t& handle)
{
    handle = port.nativeHandle;
    return port.available;
}

TEST(VideoPortHandleResolverTest, ResolvesConfiguredSparsePortsToNativeHandles)
{
    const std::vector<VideoPortHandleTestPort> ports {
        { 1, 0, 100, true }, { 1, 3, 103, true }, { 2, 7, 207, true }
    };

    for (const auto& port : ports) {
        intptr_t halHandle = 0;
        VideoPortHandleTestPort resolved {};
        ASSERT_TRUE(DeviceSettingsHAL::ResolveVideoPortHandle(
            static_cast<int32_t>(port.nativeHandle), ports, GetVideoPortHandleForTest,
            halHandle, resolved));
        EXPECT_EQ(port.nativeHandle, halHandle);
        EXPECT_EQ(port.videoPortType, resolved.videoPortType);
        EXPECT_EQ(port.videoPortIndex, resolved.videoPortIndex);
    }
}

TEST(VideoPortHandleResolverTest, RejectsUnknownAndUnavailableHandlesWithoutChangingOutputs)
{
    const std::vector<VideoPortHandleTestPort> ports {
        { 1, 0, 100, true }, { 1, 1, 101, false }
    };

    for (const int32_t handle : { 0, -1, 102, 100 }) {
        intptr_t halHandle = 999;
        VideoPortHandleTestPort resolved { 9, 9, 999, true };
        EXPECT_FALSE(DeviceSettingsHAL::ResolveVideoPortHandle(
            handle, ports, GetVideoPortHandleForTest, halHandle, resolved));
        EXPECT_EQ(999, halHandle);
        EXPECT_EQ(9, resolved.videoPortType);
        EXPECT_EQ(9, resolved.videoPortIndex);
    }
}

TEST(VideoPortHandleResolverTest, PreservesFullWidthNativeHandle)
{
    if (sizeof(intptr_t) <= sizeof(int32_t)) {
        GTEST_SKIP();
    }

    const intptr_t nativeHandle = static_cast<intptr_t>(0x1234567887654321ULL);
    const std::vector<VideoPortHandleTestPort> ports {
        { 1, 2, nativeHandle, true }
    };
    intptr_t halHandle = 0;
    VideoPortHandleTestPort resolved {};
    ASSERT_TRUE(DeviceSettingsHAL::ResolveVideoPortHandle(
        static_cast<int32_t>(nativeHandle), ports, GetVideoPortHandleForTest,
        halHandle, resolved));
    EXPECT_EQ(nativeHandle, halHandle);
    EXPECT_NE(static_cast<intptr_t>(static_cast<int32_t>(nativeHandle)), halHandle);
}

TEST(VideoPortHandleResolverTest, RejectsTruncatedHandleCollisionsInEitherOrder)
{
    if (sizeof(intptr_t) <= sizeof(int32_t)) {
        GTEST_SKIP();
    }

    const VideoPortHandleTestPort first { 1, 0, static_cast<intptr_t>(0x100000123ULL), true };
    const VideoPortHandleTestPort second { 1, 1, static_cast<intptr_t>(0x200000123ULL), true };
    for (const auto& ports : { std::vector<VideoPortHandleTestPort> { first, second },
             std::vector<VideoPortHandleTestPort> { second, first } }) {
        intptr_t halHandle = 999;
        VideoPortHandleTestPort resolved {};
        EXPECT_FALSE(DeviceSettingsHAL::ResolveVideoPortHandle(
            static_cast<int32_t>(first.nativeHandle), ports, GetVideoPortHandleForTest,
            halHandle, resolved));
        EXPECT_EQ(999, halHandle);
    }
}

TEST(VideoPortHandleResolverTest, RejectsOneNativeHandleAssignedToDifferentPorts)
{
    const std::vector<VideoPortHandleTestPort> ports {
        { 1, 0, 100, true }, { 2, 1, 100, true }
    };
    intptr_t halHandle = 999;
    VideoPortHandleTestPort resolved {};
    EXPECT_FALSE(DeviceSettingsHAL::ResolveVideoPortHandle(
        100, ports, GetVideoPortHandleForTest, halHandle, resolved));
    EXPECT_EQ(999, halHandle);
}

struct AudioPortHandleTestPort {
    int audioPortType;
    int audioPortIndex;
    intptr_t nativeHandle;
};

TEST(AudioPortHandleResolverTest, ResolvesConfiguredTypesAndSparseIndices)
{
    const std::vector<AudioPortHandleTestPort> ports {
        { 1, 0, 100 }, { 1, 3, 103 }, { 2, 7, 207 }
    };
    for (const auto& port : ports) {
        intptr_t handle = 0;
        AudioPortHandleTestPort resolved {};
        ASSERT_TRUE(DeviceSettingsHAL::ResolveAudioPortHandle(
            static_cast<int32_t>(port.nativeHandle), ports,
            [](const AudioPortHandleTestPort& candidate, intptr_t& nativeHandle) {
                nativeHandle = candidate.nativeHandle;
                return true;
            }, handle, resolved));
        EXPECT_EQ(port.nativeHandle, handle);
        EXPECT_EQ(port.audioPortType, resolved.audioPortType);
        EXPECT_EQ(port.audioPortIndex, resolved.audioPortIndex);
    }
}

TEST(AudioPortHandleResolverTest, RejectsUnknownAmbiguousAndUnavailableHandles)
{
    const std::vector<std::vector<AudioPortHandleTestPort>> configurations {
        {}, { { 1, 0, 101 } }, { { 1, 0, 100 }, { 2, 0, 100 } }
    };
    for (const auto& ports : configurations) {
        intptr_t handle = 999;
        AudioPortHandleTestPort resolved { 9, 9, 999 };
        EXPECT_FALSE(DeviceSettingsHAL::ResolveAudioPortHandle(100, ports,
            [](const AudioPortHandleTestPort& port, intptr_t& nativeHandle) {
                nativeHandle = port.nativeHandle;
                return true;
            }, handle, resolved));
        EXPECT_EQ(999, handle);
        EXPECT_EQ(9, resolved.audioPortType);
    }

    intptr_t handle = 999;
    AudioPortHandleTestPort resolved { 9, 9, 999 };
    EXPECT_FALSE(DeviceSettingsHAL::ResolveAudioPortHandle(100,
        std::vector<AudioPortHandleTestPort> { { 1, 0, 100 }, { 1, 1, 101 } },
        [](const AudioPortHandleTestPort& port, intptr_t& nativeHandle) {
            nativeHandle = port.nativeHandle;
            return port.audioPortIndex == 0;
        }, handle, resolved));
    EXPECT_EQ(999, handle);
    EXPECT_EQ(9, resolved.audioPortType);
}

TEST(AudioPortHandleResolverTest, PreservesNativeWidthAndRejectsTruncationCollisions)
{
    if (sizeof(intptr_t) <= sizeof(int32_t)) {
        GTEST_SKIP();
    }
    const AudioPortHandleTestPort first { 1, 2, static_cast<intptr_t>(0x1234567887654321ULL) };
    const AudioPortHandleTestPort second { 1, 3, static_cast<intptr_t>(0x2234567887654321ULL) };
    const auto getHandle = [](const AudioPortHandleTestPort& port, intptr_t& nativeHandle) {
        nativeHandle = port.nativeHandle;
        return true;
    };
    intptr_t handle = 0;
    AudioPortHandleTestPort resolved {};
    ASSERT_TRUE(DeviceSettingsHAL::ResolveAudioPortHandle(static_cast<int32_t>(first.nativeHandle),
        std::vector<AudioPortHandleTestPort> { first }, getHandle, handle, resolved));
    EXPECT_EQ(first.nativeHandle, handle);
    for (const auto& ports : { std::vector<AudioPortHandleTestPort> { first, second },
             std::vector<AudioPortHandleTestPort> { second, first } }) {
        handle = 999;
        EXPECT_FALSE(DeviceSettingsHAL::ResolveAudioPortHandle(
            static_cast<int32_t>(first.nativeHandle), ports, getHandle, handle, resolved));
        EXPECT_EQ(999, handle);
    }
}

TEST(AudioPortHandleCacheTest, ReusesNativeHandlesUntilClearedAndRepopulated)
{
    const intptr_t nativeHandle = sizeof(intptr_t) > sizeof(int32_t)
        ? static_cast<intptr_t>(0x1234567887654321ULL) : 100;
    const std::vector<AudioPortHandleTestPort> ports {
        { 1, 3, nativeHandle }, { 2, 7, 207 }
    };
    DeviceSettingsHAL::AudioPortHandleCache<AudioPortHandleTestPort> cache;
    size_t lookups = 0;
    intptr_t offset = 0;
    const auto getHandle = [&lookups, &offset](const AudioPortHandleTestPort& port, intptr_t& handle) {
        ++lookups;
        handle = port.nativeHandle + offset;
        return true;
    };
    ASSERT_TRUE(cache.Populate(ports, getHandle));
    ASSERT_EQ(ports.size(), lookups);
    offset = 10;
    for (int i = 0; i < 3; ++i) {
        intptr_t handle = 0;
        AudioPortHandleTestPort resolved {};
        ASSERT_TRUE(cache.Resolve(static_cast<int32_t>(nativeHandle), handle, resolved));
        EXPECT_EQ(nativeHandle, handle);
        EXPECT_EQ(1, resolved.audioPortType);
        EXPECT_EQ(3, resolved.audioPortIndex);
    }
    EXPECT_EQ(ports.size(), lookups);

    cache.Clear();
    intptr_t handle = 999;
    AudioPortHandleTestPort resolved { 9, 9, 999 };
    EXPECT_FALSE(cache.Resolve(static_cast<int32_t>(nativeHandle), handle, resolved));
    EXPECT_EQ(999, handle);
    EXPECT_EQ(9, resolved.audioPortType);
    ASSERT_TRUE(cache.Populate(ports, getHandle));
    EXPECT_EQ(ports.size() * 2, lookups);
    EXPECT_FALSE(cache.Resolve(static_cast<int32_t>(nativeHandle), handle, resolved));
    ASSERT_TRUE(cache.Resolve(static_cast<int32_t>(nativeHandle + offset), handle, resolved));
    EXPECT_EQ(nativeHandle + offset, handle);
}

TEST(AudioPortHandleCacheTest, DiscardsOldAndPartialCacheOnFailedPopulationAndAllowsRetry)
{
    const std::vector<AudioPortHandleTestPort> ports {
        { 1, 0, 100 }, { 1, 1, 101 }
    };
    DeviceSettingsHAL::AudioPortHandleCache<AudioPortHandleTestPort> cache;
    bool failSecondPort = false;
    const auto getHandle = [&failSecondPort](const AudioPortHandleTestPort& port, intptr_t& handle) {
        handle = port.nativeHandle;
        return !failSecondPort || port.audioPortIndex == 0;
    };
    ASSERT_TRUE(cache.Populate(ports, getHandle));
    failSecondPort = true;
    EXPECT_FALSE(cache.Populate(ports, getHandle));
    EXPECT_FALSE(cache.IsReady());
    intptr_t handle = 999;
    AudioPortHandleTestPort resolved { 9, 9, 999 };
    EXPECT_FALSE(cache.Resolve(100, handle, resolved));
    EXPECT_EQ(999, handle);
    EXPECT_EQ(9, resolved.audioPortType);

    failSecondPort = false;
    ASSERT_TRUE(cache.Populate(ports, getHandle));
    ASSERT_TRUE(cache.Resolve(101, handle, resolved));
    EXPECT_EQ(101, handle);
    EXPECT_FALSE(cache.Populate(std::vector<AudioPortHandleTestPort> {}, getHandle));
    EXPECT_FALSE(cache.IsReady());
    EXPECT_FALSE(cache.Resolve(101, handle, resolved));
}

TEST(AudioPortHandleCacheTest, RejectsCachedNativeAndTruncatedHandleAliases)
{
    std::vector<std::vector<AudioPortHandleTestPort>> configurations {
        { { 1, 0, 100 }, { 2, 0, 100 } }
    };
    if (sizeof(intptr_t) > sizeof(int32_t)) {
        configurations.push_back({
            { 1, 0, static_cast<intptr_t>(0x100000123ULL) },
            { 1, 1, static_cast<intptr_t>(0x200000123ULL) }
        });
    }
    for (const auto& ports : configurations) {
        DeviceSettingsHAL::AudioPortHandleCache<AudioPortHandleTestPort> cache;
        ASSERT_TRUE(cache.Populate(ports,
            [](const AudioPortHandleTestPort& port, intptr_t& handle) {
                handle = port.nativeHandle;
                return true;
            }));
        intptr_t handle = 999;
        AudioPortHandleTestPort resolved { 9, 9, 999 };
        EXPECT_FALSE(cache.Resolve(static_cast<int32_t>(ports.front().nativeHandle), handle, resolved));
        EXPECT_EQ(999, handle);
        EXPECT_EQ(9, resolved.audioPortType);
    }
}

TEST(VideoPortHandleCacheTest, ReusesConfiguredSparsePortsUntilClearedAndRebuilt)
{
    const intptr_t nativeHandle = sizeof(intptr_t) > sizeof(int32_t)
        ? static_cast<intptr_t>(0x1234567887654321ULL) : 103;
    const std::vector<VideoPortHandleTestPort> ports {
        { 1, 3, nativeHandle, true }, { 2, 7, 207, true }
    };
    DeviceSettingsHAL::VideoPortHandleCache<VideoPortHandleTestPort> cache;
    size_t lookups = 0;
    intptr_t offset = 0;
    const auto getHandle = [&lookups, &offset](const VideoPortHandleTestPort& port, intptr_t& handle) {
        ++lookups;
        handle = port.nativeHandle + offset;
        return port.available;
    };
    ASSERT_TRUE(cache.Populate(ports, getHandle));
    for (int repeat = 0; repeat < 3; ++repeat) {
        for (const auto& port : ports) {
            intptr_t handle = 0;
            VideoPortHandleTestPort resolved {};
            ASSERT_TRUE(cache.Resolve(static_cast<int32_t>(port.nativeHandle), handle, resolved));
            EXPECT_EQ(port.nativeHandle, handle);
            EXPECT_EQ(port.videoPortType, resolved.videoPortType);
            EXPECT_EQ(port.videoPortIndex, resolved.videoPortIndex);
        }
    }
    EXPECT_EQ(ports.size(), lookups);

    cache.Clear();
    intptr_t handle = 999;
    VideoPortHandleTestPort resolved { 9, 9, 999, true };
    EXPECT_FALSE(cache.Resolve(static_cast<int32_t>(nativeHandle), handle, resolved));
    EXPECT_EQ(999, handle);
    EXPECT_EQ(9, resolved.videoPortType);
    offset = 10;
    ASSERT_TRUE(cache.Populate(ports, getHandle));
    EXPECT_FALSE(cache.Resolve(static_cast<int32_t>(nativeHandle), handle, resolved));
    ASSERT_TRUE(cache.Resolve(static_cast<int32_t>(nativeHandle + offset), handle, resolved));
    EXPECT_EQ(nativeHandle + offset, handle);
    EXPECT_EQ(ports.size() * 2, lookups);
}

TEST(DisplayHandleCacheTest, CachesExplicitNonSupportButDiscardsFailedLookups)
{
    using Status = DeviceSettingsHAL::HandleLookupResult;
    const std::vector<VideoPortHandleTestPort> ports {
        { 1, 3, 103, true }, { 2, 0, 103, false }
    };
    DeviceSettingsHAL::VideoPortHandleCache<VideoPortHandleTestPort> cache;
    size_t lookups = 0;
    Status unavailableStatus = Status::Unsupported;
    const auto getHandle = [&lookups, &unavailableStatus](const VideoPortHandleTestPort& port, intptr_t& handle) {
        ++lookups;
        handle = port.nativeHandle;
        return port.available ? Status::Found : unavailableStatus;
    };
    ASSERT_TRUE(cache.Populate(ports, getHandle));
    for (int repeat = 0; repeat < 3; ++repeat) {
        intptr_t handle = 0;
        VideoPortHandleTestPort resolved {};
        ASSERT_TRUE(cache.Resolve(103, handle, resolved));
        EXPECT_EQ(103, handle);
        EXPECT_EQ(1, resolved.videoPortType);
    }
    EXPECT_EQ(ports.size(), lookups);

    unavailableStatus = Status::Failed;
    EXPECT_FALSE(cache.Populate(ports, getHandle));
    EXPECT_FALSE(cache.IsReady());
    intptr_t handle = 999;
    VideoPortHandleTestPort resolved { 9, 9, 999, true };
    EXPECT_FALSE(cache.Resolve(103, handle, resolved));
    EXPECT_EQ(999, handle);
    EXPECT_EQ(9, resolved.videoPortType);

    size_t unsupportedLookups = 0;
    ASSERT_TRUE(cache.Populate(ports,
        [&unsupportedLookups](const VideoPortHandleTestPort&, intptr_t&) {
            ++unsupportedLookups;
            return Status::Unsupported;
        }));
    EXPECT_TRUE(cache.IsReady());
    for (int repeat = 0; repeat < 3; ++repeat) {
        EXPECT_FALSE(cache.Resolve(103, handle, resolved));
    }
    EXPECT_EQ(ports.size(), unsupportedLookups);
}

TEST(VideoDeviceHandleCacheTest, CachesZeroAndRejectsIndicesAliasingOneNativeHandle)
{
    std::vector<intptr_t> nativeHandles { 0, 103, 207 };
    std::vector<int32_t> indices;
    ASSERT_TRUE(DeviceSettingsHAL::PopulateVideoDeviceIndices(nativeHandles.size(), indices));
    DeviceSettingsHAL::PortHandleCache<int32_t> cache;
    size_t lookups = 0;
    const auto getHandle = [&lookups, &nativeHandles](const int32_t index, intptr_t& handle) {
        ++lookups;
        handle = nativeHandles.at(index);
        return true;
    };
    ASSERT_TRUE(cache.Populate(indices, getHandle));
    for (const auto expected : indices) {
        intptr_t handle = 999;
        int32_t index = -1;
        ASSERT_TRUE(cache.Resolve(static_cast<int32_t>(nativeHandles.at(expected)), handle, index));
        EXPECT_EQ(nativeHandles.at(expected), handle);
        EXPECT_EQ(expected, index);
    }
    EXPECT_EQ(indices.size(), lookups);

    nativeHandles[1] = 0;
    cache.Clear();
    ASSERT_TRUE(cache.Populate(indices, getHandle));
    intptr_t handle = 999;
    int32_t index = -1;
    EXPECT_FALSE(cache.Resolve(0, handle, index));
    EXPECT_EQ(999, handle);
    EXPECT_EQ(-1, index);
    ASSERT_TRUE(cache.Resolve(207, handle, index));
    EXPECT_EQ(207, handle);
    EXPECT_EQ(2, index);
    EXPECT_EQ(indices.size() * 2, lookups);
}

TEST(DisplayHandleResolverTest, SkipsExplicitNonSupportButRejectsLookupFailure)
{
    using Status = DeviceSettingsHAL::HandleLookupResult;
    const VideoPortHandleTestPort valid { 1, 3, 103, true };
    const VideoPortHandleTestPort other { 2, 0, 103, false };
    for (const auto& ports : { std::vector<VideoPortHandleTestPort> { valid, other },
             std::vector<VideoPortHandleTestPort> { other, valid } }) {
        intptr_t handle = 0;
        VideoPortHandleTestPort resolved {};
        ASSERT_TRUE(DeviceSettingsHAL::ResolveVideoPortHandle(103, ports,
            [](const VideoPortHandleTestPort& port, intptr_t& nativeHandle) {
                nativeHandle = port.nativeHandle;
                return port.available ? Status::Found : Status::Unsupported;
            }, handle, resolved));
        EXPECT_EQ(103, handle);
        EXPECT_EQ(1, resolved.videoPortType);

        handle = 999;
        EXPECT_FALSE(DeviceSettingsHAL::ResolveVideoPortHandle(103, ports,
            [](const VideoPortHandleTestPort& port, intptr_t& nativeHandle) {
                nativeHandle = port.nativeHandle;
                return port.available ? Status::Found : Status::Failed;
            }, handle, resolved));
        EXPECT_EQ(999, handle);
    }
    intptr_t handle = 999;
    VideoPortHandleTestPort resolved {};
    EXPECT_FALSE(DeviceSettingsHAL::ResolveVideoPortHandle(103,
        std::vector<VideoPortHandleTestPort> { other },
        [](const VideoPortHandleTestPort&, intptr_t&) { return Status::Unsupported; },
        handle, resolved));
    EXPECT_EQ(999, handle);
}

TEST(VideoDeviceHandleResolverTest, AcceptsZeroHandleAndResolvesEveryConfiguredIndex)
{
    const std::vector<intptr_t> nativeHandles { 0, 103, 207 };
    for (int32_t expected = 0; expected < 3; ++expected) {
        intptr_t handle = 999;
        int32_t index = -1;
        ASSERT_TRUE(DeviceSettingsHAL::ResolveVideoDeviceHandle(
            static_cast<int32_t>(nativeHandles[expected]), nativeHandles.size(),
            [&nativeHandles](int32_t candidateIndex, intptr_t& candidate) {
                candidate = nativeHandles.at(candidateIndex);
                return true;
            }, handle, index));
        EXPECT_EQ(nativeHandles[expected], handle);
        EXPECT_EQ(expected, index);
    }
}

TEST(VideoDeviceHandleResolverTest, RejectsUnknownAliasesEmptyConfigurationAndFailedLookups)
{
    for (const size_t count : { size_t(0), size_t(1), size_t(2) }) {
        intptr_t handle = 999;
        int32_t index = 9;
        EXPECT_FALSE(DeviceSettingsHAL::ResolveVideoDeviceHandle(100, count,
            [](int32_t, intptr_t& candidate) { candidate = 200; return true; }, handle, index));
        EXPECT_EQ(999, handle);
        EXPECT_EQ(9, index);
    }
    intptr_t handle = 999;
    int32_t index = 9;
    EXPECT_FALSE(DeviceSettingsHAL::ResolveVideoDeviceHandle(0, 2,
        [](int32_t, intptr_t& candidate) { candidate = 0; return true; }, handle, index));
    EXPECT_EQ(999, handle);
    EXPECT_EQ(9, index);
    EXPECT_FALSE(DeviceSettingsHAL::ResolveVideoDeviceHandle(0, 2,
        [](int32_t candidateIndex, intptr_t& candidate) {
            candidate = 0;
            return candidateIndex == 0;
        }, handle, index));
    EXPECT_EQ(999, handle);
    EXPECT_EQ(9, index);
}

} // namespace