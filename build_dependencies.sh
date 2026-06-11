#!/bin/bash
set -x
set -e

GITHUB_WORKSPACE="${PWD}"
ls -la "${GITHUB_WORKSPACE}"
cd "${GITHUB_WORKSPACE}"

apt update
apt install -y libcurl4-openssl-dev valgrind lcov clang libsystemd-dev libboost-all-dev meson curl libunwind-dev libdrm-dev
pip install jsonref

git clone --branch R4.4.3 https://github.com/rdkcentral/ThunderTools.git
git clone --branch R4.4.1 https://github.com/rdkcentral/Thunder.git
git clone --branch feature/RDKEMW-6078_DeviceSettings_Interface https://github.com/rdkcentral/entservices-apis.git
git clone --branch 1.0.14 https://github.com/rdkcentral/entservices-testframework.git

echo "======================================================================================"
echo "building thunderTools"
cd ThunderTools
patch -p1 < "$GITHUB_WORKSPACE/entservices-testframework/patches/00010-R4.4-Add-support-for-project-dir.patch"
cd -

cmake -G Ninja -S ThunderTools -B build/ThunderTools \
    -DEXCEPTIONS_ENABLE=ON \
    -DCMAKE_INSTALL_PREFIX="$GITHUB_WORKSPACE/install/usr" \
    -DCMAKE_MODULE_PATH="$GITHUB_WORKSPACE/install/tools/cmake" \
    -DGENERIC_CMAKE_MODULE_PATH="$GITHUB_WORKSPACE/install/tools/cmake"

cmake --build build/ThunderTools --target install

echo "======================================================================================"
echo "building thunder"
cd Thunder
patch -p1 < "$GITHUB_WORKSPACE/entservices-testframework/patches/Use_Legact_Alt_Based_On_ThunderTools_R4.4.3.patch"
patch -p1 < "$GITHUB_WORKSPACE/entservices-testframework/patches/error_code_R4_4.patch"
patch -p1 < "$GITHUB_WORKSPACE/entservices-testframework/patches/1004-Add-support-for-project-dir.patch"
patch -p1 < "$GITHUB_WORKSPACE/entservices-testframework/patches/RDKEMW-733-Add-ENTOS-IDS.patch"
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

echo "======================================================================================"
echo "generating mock DeviceSettings HAL headers"
mkdir -p "$GITHUB_WORKSPACE/entservices-testframework/Tests/headers/rdk/ds"
mkdir -p "$GITHUB_WORKSPACE/install/usr/include/rdk/ds"
mkdir -p "$GITHUB_WORKSPACE/install/usr/include/rdk/halif/ds-hal"
mkdir -p "$GITHUB_WORKSPACE/install/usr/include/rdk/ds-rpc"

cd "$GITHUB_WORKSPACE/entservices-testframework/Tests/headers/rdk/ds"

touch dsMgr.h
touch dsTypes.h
touch dsUtl.h
touch dsError.h
touch dsRpc.h
touch dsDisplay.h
touch dsVideoPort.h
touch dsVideoDevice.h
touch dsAudio.h
touch dsHdmiIn.h
touch dsFPD.h
touch dsFPDTypes.h
touch dsCompositeIn.h
touch exception.hpp
touch hdmiIn.hpp
touch host.hpp
touch list.hpp
touch manager.hpp
touch sleepMode.hpp
touch videoDevice.hpp
touch videoOutputPort.hpp
touch videoOutputPortConfig.hpp
touch videoOutputPortType.hpp
touch videoResolution.hpp
touch audioOutputPort.hpp
touch audioOutputPortType.hpp
touch audioOutputPortConfig.hpp
touch compositeIn.hpp
touch pixelResolution.hpp
touch frontPanelIndicator.hpp
touch frontPanelConfig.hpp
touch frontPanelTextDisplay.hpp

# Also create headers in install directory for CMake FindDS.cmake
cd "$GITHUB_WORKSPACE/install/usr/include/rdk/ds"

touch dsMgr.h
touch dsTypes.h
touch dsUtl.h
touch dsError.h
touch dsRpc.h
touch dsDisplay.h
touch dsVideoPort.h
touch dsVideoDevice.h
touch dsAudio.h
touch dsHdmiIn.h
touch dsFPD.h
touch dsFPDTypes.h
touch dsCompositeIn.h
touch exception.hpp
touch hdmiIn.hpp
touch host.hpp
touch list.hpp
touch manager.hpp
touch sleepMode.hpp
touch videoDevice.hpp
touch videoOutputPort.hpp
touch videoOutputPortConfig.hpp
touch videoOutputPortType.hpp
touch videoResolution.hpp
touch audioOutputPort.hpp
touch audioOutputPortType.hpp
touch audioOutputPortConfig.hpp
touch compositeIn.hpp
touch pixelResolution.hpp
touch frontPanelIndicator.hpp
touch frontPanelConfig.hpp
touch frontPanelTextDisplay.hpp

# Create HAL headers
touch "$GITHUB_WORKSPACE/install/usr/include/rdk/halif/ds-hal/dsTypes.h"

# Create RPC headers
touch "$GITHUB_WORKSPACE/install/usr/include/rdk/ds-rpc/dsMgr.h"

cd "$GITHUB_WORKSPACE"

echo "======================================================================================"
echo "device-settings repository dependencies are ready"
