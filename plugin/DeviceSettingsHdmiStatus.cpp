/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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

#include "Module.h"
#include "DeviceSettingsHdmiStatus.h"

#include "dsDisplay.h"
#include "dsVideoPort.h"

#include <wpeframework/helpers/UtilsLogging.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

struct HdmiEnumToStrMapping
{
    int tag;
    const char* name;
};

static HdmiEnumToStrMapping HdmiConnectionToStrMapping[] = {
    {dsDISPLAY_EVENT_CONNECTED, "CONNECTED"},
    {dsDISPLAY_EVENT_DISCONNECTED, "DISCONNECTED"},
    {0, 0}
};

static HdmiEnumToStrMapping HdmiStatusToStrMapping[] = {
    {dsHDCP_STATUS_UNPOWERED, "UNPOWERED"},
    {dsHDCP_STATUS_UNAUTHENTICATED, "UNAUTHENTICATED"},
    {dsHDCP_STATUS_AUTHENTICATED, "AUTHENTICATED"},
    {dsHDCP_STATUS_AUTHENTICATIONFAILURE, "AUTHENTICATIONFAILURE"},
    {dsHDCP_STATUS_INPROGRESS, "INPROGRESS"},
    {dsHDCP_STATUS_PORTDISABLED, "PORTDISABLED"},
    {0, 0}
};

static HdmiEnumToStrMapping HdmiVerToStrMapping[] = {
    {dsHDCP_VERSION_1X, "VERSION_1X"},
    {dsHDCP_VERSION_2X, "VERSION_2X"},
    {0, 0}
};

static std::string getHdmiConnectionName(const int key)
{
    int i = 0;
    while (HdmiConnectionToStrMapping[i].name) {
        if (HdmiConnectionToStrMapping[i].tag == key)
            return HdmiConnectionToStrMapping[i].name;
        i++;
    }
    return "";
}

static std::string getHdcpStatusName(const int key)
{
    int i = 0;
    while (HdmiStatusToStrMapping[i].name) {
        if (HdmiStatusToStrMapping[i].tag == key)
            return HdmiStatusToStrMapping[i].name;
        i++;
    }
    return "";
}

static std::string getHdcpVersionName(const int key)
{
    int i = 0;
    while (HdmiVerToStrMapping[i].name) {
        if (HdmiVerToStrMapping[i].tag == key)
            return HdmiVerToStrMapping[i].name;
        i++;
    }
    return "";
}

void _dsSyncHdmiStatus(const std::string& key, int val)
{
    std::vector<std::string> lines;
    std::ifstream statusFile(DS_HDMI_STATUS_FILE);
    std::string line;
    bool found = false;
    std::string value = "";

    if (0 == strncmp(key.c_str(), DS_HDMI_TAG_HOTPLUP, strlen(DS_HDMI_TAG_HOTPLUP))) {
        value = getHdmiConnectionName(val);
    } else if (0 == strncmp(key.c_str(), DS_HDMI_TAG_HDCPSTATUS, strlen(DS_HDMI_TAG_HDCPSTATUS))) {
        value = getHdcpStatusName(val);
    } else if (0 == strncmp(key.c_str(), DS_HDMI_TAG_HDCPVERSION, strlen(DS_HDMI_TAG_HDCPVERSION))) {
        value = getHdcpVersionName(val);
    } else {
        LOGWARN("_dsSyncHdmiStatus: unknown key is passed %s", key.c_str());
    }

    while (std::getline(statusFile, line)) {
        if (line.find(key + "=") == 0) {
            found = true;
        } else {
            lines.push_back(line);
        }
    }
    statusFile.close();

    lines.push_back(key + "=" + value);

    std::ofstream outFile(DS_HDMI_STATUS_FILE);
    for (const auto& statusLine : lines) {
        outFile << statusLine << std::endl;
    }
    outFile.close();

    if (found) {
        LOGINFO("Updated %s to %s", key.c_str(), value.c_str());
    } else {
        LOGINFO("Added %s with value %s", key.c_str(), value.c_str());
    }
}
