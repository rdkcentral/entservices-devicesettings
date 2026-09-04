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

#pragma once

#include <string>

#define DS_HDMI_STATUS_FILE "/tmp/ds_hdmi_status.bin"
#define DS_HDMI_TAG_HOTPLUP "hotplug"
#define DS_HDMI_TAG_HDCPSTATUS "hdcp_status"
#define DS_HDMI_TAG_HDCPVERSION "hdcp_version"

void _dsSyncHdmiStatus(const std::string& key, int val);
