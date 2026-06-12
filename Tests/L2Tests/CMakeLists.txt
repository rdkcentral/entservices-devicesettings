# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
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

set(PLUGIN_NAME L2TestsDS)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(THUNDER_PORT 9998)

find_package(${NAMESPACE}Plugins REQUIRED)

set(SRC_FILES
    tests/DeviceSettings_L2Test.cpp
)

add_library(${MODULE_NAME} SHARED ${SRC_FILES})

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME}
            PRIVATE
            MODULE_NAME=Plugin_${PLUGIN_NAME}
            THUNDER_PORT="${THUNDER_PORT}")

target_compile_options(${MODULE_NAME} PRIVATE -Wno-error)
target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

target_include_directories(
    ${MODULE_NAME} PRIVATE ./
    ../../plugin/DeviceSettings
    ../../../entservices-testframework/Tests/mocks
    ../../../entservices-testframework/Tests/mocks/thunder
    ../../../entservices-testframework/Tests/mocks/devicesettings
    ../../../entservices-testframework/Tests/mocks/MockPlugin
    ../../../entservices-testframework/Tests/L2Tests/L2TestsPlugin
    ${CMAKE_INSTALL_PREFIX}/include
    )

install(TARGETS ${MODULE_NAME} DESTINATION lib)