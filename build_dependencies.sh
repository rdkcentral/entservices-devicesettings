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
cd "$GITHUB_WORKSPACE"
cd entservices-testframework/Tests
echo "Empty mocks creation to avoid compilation errors"
echo "======================================================================================"
mkdir -p headers
mkdir -p headers/audiocapturemgr
mkdir -p headers/rdk/ds
mkdir -p headers/rdk/iarmbus
mkdir -p headers/rdk/iarmmgrs-hal
mkdir -p headers/ccec/drivers
mkdir -p headers/network
echo "dir created successfully"
echo "======================================================================================"

echo "======================================================================================"
echo "empty headers creation"
cd headers

DS_MOCK_HEADERS="
dsMgr.h
dsTypes.h
dsUtl.h
dsError.h
dsRpc.h
dsDisplay.h
dsVideoPort.h
dsVideoDevice.h
dsAudio.h
dsHdmiIn.h
dsHdmiInTypes.h
dsFPD.h
dsFPDTypes.h
dsCompositeIn.h
dsHost.h
exception.hpp
hdmiIn.hpp
host.hpp
list.hpp
manager.hpp
sleepMode.hpp
videoDevice.hpp
videoOutputPort.hpp
videoOutputPortConfig.hpp
videoOutputPortType.hpp
videoResolution.hpp
audioOutputPort.hpp
audioOutputPortType.hpp
audioOutputPortConfig.hpp
compositeIn.hpp
pixelResolution.hpp
frontPanelIndicator.hpp
frontPanelConfig.hpp
frontPanelTextDisplay.hpp
"

for DEST in \
    "$GITHUB_WORKSPACE/entservices-testframework/Tests/headers/rdk/ds" \
    "$GITHUB_WORKSPACE/entservices-testframework/Tests/headers"; do
    printf '%s\n' "$DS_MOCK_HEADERS" | while IFS= read -r HEADER; do
        if [ -n "$HEADER" ]; then
            touch "$DEST/$HEADER"
        fi
    done
done

cd "$GITHUB_WORKSPACE"

echo "======================================================================================"
echo "device-settings repository dependencies are ready"
