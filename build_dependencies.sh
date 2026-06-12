#!/bin/bash
set -x
set -e

GITHUB_WORKSPACE="${PWD}"
ls -la "${GITHUB_WORKSPACE}"
cd "${GITHUB_WORKSPACE}"

apt update
apt install -y libsqlite3-dev libcurl4-openssl-dev valgrind lcov clang libsystemd-dev libboost-all-dev libwebsocketpp-dev meson libcunit1 libcunit1-dev curl protobuf-compiler-grpc libgrpc-dev libgrpc++-dev libunwind-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libdrm-dev
pip install jsonref

if [ ! -d "trower-base64" ]; then
    git clone https://github.com/xmidt-org/trower-base64.git
fi
cd trower-base64
meson setup --warnlevel 3 --werror build
ninja -C build
ninja -C build install
cd ..

git clone --branch R4.4.3 https://github.com/rdkcentral/ThunderTools.git
git clone --branch R4.4.1 https://github.com/rdkcentral/Thunder.git
git clone --branch feature/RDKEMW-6078_DeviceSettings_Interface https://github.com/rdkcentral/entservices-apis.git
git clone --branch 1.0.14 https://github.com/rdkcentral/entservices-testframework.git
git clone --branch main https://github.com/rdkcentral/rdk-halif-device_settings.git
git clone --branch main https://github.com/rdkcentral/devicesettings.git
git clone --branch develop https://github.com/rdkcentral/iarmbus.git
git clone https://github.com/rdkcentral/iarmmgrs.git

# Ensure mock iarmmgrs-hal headers exist in testframework for CI builds.
mkdir -p "$GITHUB_WORKSPACE/entservices-testframework/Tests/headers/rdk/iarmmgrs-hal"
touch "$GITHUB_WORKSPACE/entservices-testframework/Tests/headers/rdk/iarmmgrs-hal/sysMgr.h"
touch "$GITHUB_WORKSPACE/entservices-testframework/Tests/headers/rdk/iarmmgrs-hal/mfrMgr.h"

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

mkdir -p "$GITHUB_WORKSPACE/install/usr/include/WPEFramework/interfaces"
find "$GITHUB_WORKSPACE/entservices-apis/apis/DeviceSettings" -name "IDeviceSettings*.h" -exec cp {} "$GITHUB_WORKSPACE/install/usr/include/WPEFramework/interfaces/" \; 2>/dev/null || true

cp -r "$GITHUB_WORKSPACE/rdk-halif-device_settings/include/." "$GITHUB_WORKSPACE/install/usr/include/"
cp -r "$GITHUB_WORKSPACE/devicesettings/rpc/include/." "$GITHUB_WORKSPACE/install/usr/include/"
cp -r "$GITHUB_WORKSPACE/devicesettings/ds/include/." "$GITHUB_WORKSPACE/install/usr/include/"

# Real IARM headers from iarmbus repo
cp -r "$GITHUB_WORKSPACE/iarmbus/core/include/." "$GITHUB_WORKSPACE/install/usr/include/"

# Create stub headers for external dependencies with no public repos
touch "$GITHUB_WORKSPACE/install/usr/include/rfcapi.h"
touch "$GITHUB_WORKSPACE/install/usr/include/mfrMgr.h"
touch "$GITHUB_WORKSPACE/install/usr/include/secure_wrapper.h"

# Copy real iarmmgrs public headers used by DeviceSettings.
cp "$GITHUB_WORKSPACE/iarmmgrs/sysmgr/include/sysMgr.h" "$GITHUB_WORKSPACE/install/usr/include/"

# Copy external stubs from testframework (no public repos available)
find "$GITHUB_WORKSPACE/entservices-testframework/Tests/headers" -maxdepth 1 -type f -name "*.h" -exec cp {} "$GITHUB_WORKSPACE/install/usr/include/" \; 2>/dev/null || true
find "$GITHUB_WORKSPACE/entservices-testframework/Tests/headers/ccec" -maxdepth 1 -type f -name "*.h" -exec cp {} "$GITHUB_WORKSPACE/install/usr/include/" \; 2>/dev/null || true
find "$GITHUB_WORKSPACE/entservices-testframework/Tests/headers/rdk/iarmmgrs-hal" -maxdepth 1 -type f -name "*.h" -exec cp {} "$GITHUB_WORKSPACE/install/usr/include/" \; 2>/dev/null || true

# Ensure real iarmmgrs headers take precedence after external stub copies.
cp "$GITHUB_WORKSPACE/iarmmgrs/sysmgr/include/sysMgr.h" "$GITHUB_WORKSPACE/install/usr/include/"

echo "======================================================================================"
echo "device-settings repository dependencies are ready"