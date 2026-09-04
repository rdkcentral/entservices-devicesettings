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

// Mirrors lib32-iarmmgrs dsmgr's utils/iarmutilslogger.h: the legacy dsmgr
// daemon defines these macros itself against <telemetry_busmessage_sender.h>
// rather than including rdk/ds-rpc/dsTelemetry.h (that header is only used by
// the separate rpc/srv library, not the daemon this plugin replaces).
#include <telemetry_busmessage_sender.h>

#define TELEMETRY_INIT(component) \
    do { \
        t2_init((char*)component); \
    } while(0)

#define TELEMETRY_UNINIT() \
    do { \
        t2_uninit(); \
    } while(0)

#define TELEMETRY_EVENT_STRING(marker, value) \
    do { \
        t2_event_s((char*)marker, (char*)value); \
    } while(0)

#define TELEMETRY_EVENT_FLOAT(marker, value) \
    do { \
        t2_event_f((char*)marker, (double)value); \
    } while(0)

#define TELEMETRY_EVENT_INT(marker, value) \
    do { \
        t2_event_d((char*)marker, (int)value); \
    } while(0)
