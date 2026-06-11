#!/bin/bash
#
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2026 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -x
set -e
##############################
GITHUB_WORKSPACE="${PWD}"
ls -la ${GITHUB_WORKSPACE}
cd ${GITHUB_WORKSPACE}

# ############################# 
#1. Install Dependencies and packages

apt update
apt install -y valgrind lcov clang libsystemd-dev meson curl libunwind-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libboost-all-dev libcurl4-openssl-dev libdrm-dev
pip install jsonref

############################
# Build trower-base64
if [ ! -d "trower-base64" ]; then
git clone https://github.com/xmidt-org/trower-base64.git
fi
cd trower-base64
meson setup --warnlevel 3 --werror build
ninja -C build
sudo ninja -C build install
cd ..

###########################################
# Clone the required repositories

git clone --branch R4.4.3 https://github.com/rdkcentral/ThunderTools.git

git clone --branch R4.4.1 https://github.com/rdkcentral/Thunder.git

git clone --branch feature/RDKEMW-6078_DeviceSettings_Interface https://github.com/rdkcentral/entservices-apis.git

git clone --branch 1.0.14 https://github.com/rdkcentral/entservices-testframework.git

############################
# Build Thunder-Tools
echo "======================================================================================"
echo "building thunderTools"
cd ThunderTools
patch -p1 < $GITHUB_WORKSPACE/entservices-testframework/patches/00010-R4.4-Add-support-for-project-dir.patch
cd -

cmake -G Ninja -S ThunderTools -B build/ThunderTools \
    -DEXCEPTIONS_ENABLE=ON \
    -DCMAKE_INSTALL_PREFIX="$GITHUB_WORKSPACE/install/usr" \
    -DCMAKE_MODULE_PATH="$GITHUB_WORKSPACE/install/tools/cmake" \
    -DGENERIC_CMAKE_MODULE_PATH="$GITHUB_WORKSPACE/install/tools/cmake"

cmake --build build/ThunderTools --target install

############################
# Build Thunder
echo "======================================================================================"
echo "building thunder"

cd Thunder
patch -p1 < $GITHUB_WORKSPACE/entservices-testframework/patches/Use_Legact_Alt_Based_On_ThunderTools_R4.4.3.patch
patch -p1 < $GITHUB_WORKSPACE/entservices-testframework/patches/error_code_R4_4.patch
patch -p1 < $GITHUB_WORKSPACE/entservices-testframework/patches/1004-Add-support-for-project-dir.patch
patch -p1 < $GITHUB_WORKSPACE/entservices-testframework/patches/RDKEMW-733-Add-ENTOS-IDS.patch
cd -

cmake -G Ninja -S Thunder -B build/Thunder \
    -DMESSAGING=ON \
    -DCMAKE_INSTALL_PREFIX="$GITHUB_WORKSPACE/install/usr" \
    -DCMAKE_MODULE_PATH="$GITHUB_WORKSPACE/install/tools/cmake" \
    -DGENERIC_CMAKE_MODULE_PATH="$GITHUB_WORKSPACE/install/tools/cmake" \
    -DBUILD_TYPE=Debug \
    -DBINDING=127.0.0.1 \
    -DPORT=55555 \
    -DEXCEPTIONS_ENABLE=ON

cmake --build build/Thunder --target install

############################
# Build entservices-apis
echo "======================================================================================"
echo "building entservices-apis"
cd entservices-apis
rm -rf jsonrpc/DTV.json
cd ..

cmake -G Ninja -S entservices-apis -B build/entservices-apis \
    -DEXCEPTIONS_ENABLE=ON \
    -DCMAKE_INSTALL_PREFIX="$GITHUB_WORKSPACE/install/usr" \
    -DCMAKE_MODULE_PATH="$GITHUB_WORKSPACE/install/tools/cmake"

cmake --build build/entservices-apis --target install

############################
# Generating minimal mock headers
cd $GITHUB_WORKSPACE/entservices-testframework/Tests
mkdir -p headers
cd headers
touch secure_wrapper.h
touch wpa_ctrl.h
touch rdk_logger_milestone.h
mkdir -p rdk/iarmbus
touch rdk/iarmbus/libIARM.h
touch rdk/iarmbus/libIBus.h
touch iarm.h
cd $GITHUB_WORKSPACE

############################
# Generating external headers for DeviceSettings
cd $GITHUB_WORKSPACE
cd entservices-testframework/Tests
echo "Empty mocks creation to avoid compilation errors"
echo "======================================================================================"
mkdir -p headers
mkdir -p headers/rdk/ds
echo "dir created successfully"
echo "======================================================================================"

echo "======================================================================================"
echo "empty headers creation"
cd headers
echo "current working dir: "${PWD}

# Create all DS HAL headers
touch rdk/ds/dsMgr.h
touch rdk/ds/dsTypes.h
touch rdk/ds/dsUtl.h
touch rdk/ds/dsError.h
touch rdk/ds/dsRpc.h
touch rdk/ds/dsDisplay.h
touch rdk/ds/dsVideoPort.h
touch rdk/ds/dsVideoDevice.h
touch rdk/ds/dsAudio.h
touch rdk/ds/dsHdmiIn.h
touch rdk/ds/dsFPD.h
touch rdk/ds/dsFPDTypes.h
touch rdk/ds/dsCompositeIn.h
touch rdk/ds/exception.hpp
touch rdk/ds/hdmiIn.hpp
touch rdk/ds/host.hpp
touch rdk/ds/list.hpp
touch rdk/ds/manager.hpp
touch rdk/ds/sleepMode.hpp
touch rdk/ds/videoDevice.hpp
touch rdk/ds/videoOutputPort.hpp
touch rdk/ds/videoOutputPortConfig.hpp
touch rdk/ds/videoOutputPortType.hpp
touch rdk/ds/videoResolution.hpp
touch rdk/ds/audioOutputPort.hpp
touch rdk/ds/audioOutputPortType.hpp
touch rdk/ds/audioOutputPortConfig.hpp
touch rdk/ds/compositeIn.hpp
touch rdk/ds/pixelResolution.hpp
touch rdk/ds/frontPanelIndicator.hpp
touch rdk/ds/frontPanelConfig.hpp
touch rdk/ds/frontPanelTextDisplay.hpp
touch edid-parser.hpp
touch rfcapi.h

echo "files created successfully"
echo "======================================================================================"

# Also create headers in install/usr/include for FindDS.cmake
mkdir -p "$GITHUB_WORKSPACE/install/usr/include/rdk/ds"
mkdir -p "$GITHUB_WORKSPACE/install/usr/include/rdk/halif/ds-hal"
mkdir -p "$GITHUB_WORKSPACE/install/usr/include/rdk/ds-rpc"

# Copy or create DS headers in install directory
touch \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsMgr.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsTypes.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsUtl.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsError.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsRpc.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsDisplay.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsVideoPort.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsVideoDevice.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsAudio.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsHdmiIn.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsFPD.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsFPDTypes.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/dsCompositeIn.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/exception.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/hdmiIn.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/host.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/list.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/manager.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/sleepMode.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/videoDevice.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/videoOutputPort.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/videoOutputPortConfig.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/videoOutputPortType.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/videoResolution.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/audioOutputPort.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/audioOutputPortType.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/audioOutputPortConfig.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/compositeIn.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/pixelResolution.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/frontPanelIndicator.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/frontPanelConfig.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds/frontPanelTextDisplay.hpp" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/halif/ds-hal/dsTypes.h" \
    "$GITHUB_WORKSPACE/install/usr/include/rdk/ds-rpc/dsMgr.h"

echo "======================================================================================"
echo "device-settings repository dependencies are ready"
ls -la ${GITHUB_WORKSPACE}
