#!/bin/bash
set -x
set -e

GITHUB_WORKSPACE="${PWD}"
ls -la "${GITHUB_WORKSPACE}"

echo "building entservices-devicesettings"

cd "${GITHUB_WORKSPACE}"
cmake -G Ninja -S "$GITHUB_WORKSPACE" -B build/entservices-devicesettings \
    -DUSE_THUNDER_R4=ON \
    -DCMAKE_INSTALL_PREFIX="$GITHUB_WORKSPACE/install/usr" \
    -DCMAKE_MODULE_PATH="$GITHUB_WORKSPACE/install/tools/cmake" \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_IARMBus=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_RFC=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_DS=ON \
    -DCOMCAST_CONFIG=OFF \
    -DRDK_SERVICES_COVERITY=ON \
    -DHIDE_NON_EXTERNAL_SYMBOLS=OFF \
    -DPLUGIN_DEVICESETTINGS=ON \
    -DCMAKE_CXX_FLAGS="-DEXCEPTIONS_ENABLE=ON \
    -I ${GITHUB_WORKSPACE}/install/usr/include \
    -I ${GITHUB_WORKSPACE}/install/usr/include/WPEFramework \
    -I ${GITHUB_WORKSPACE}/devicesettings/ds/include \
    -Wall -Werror -Wno-error=format \
    -DUSE_THUNDER_R4=ON -DTHUNDER_VERSION=4 -DTHUNDER_VERSION_MAJOR=4 -DTHUNDER_VERSION_MINOR=4" \

cmake --build build/entservices-devicesettings --target install
echo "======================================================================================"
exit 0