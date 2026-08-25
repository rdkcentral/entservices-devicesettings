// Out-of-line virtual destructor definition for RTTI/typeinfo
/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
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

#include <cstdint>
#include <cstdio>
#include <dlfcn.h>
#include <iostream>
#include <functional>
#include <string>
#include <unordered_map>
#include "dHost.h"
#include "dsHost.h"
#include "dsError.h"
#include "dsUtl.h"
#include "dsTypes.h"

#include <WPEFramework/interfaces/IDeviceSettingsHost.h>
#include "DeviceSettingsTypes.h"



// Static global variables from dsHost.cpp conversion
static int host_isInitialized = 0;
static int host_isPlatInitialized = 0;

// MS12 Configuration constants
#ifndef MS12_CONFIG_BUF_SIZE
#define MS12_CONFIG_BUF_SIZE 256
#endif

// EDID constants
#ifndef EDID_MAX_DATA_SIZE
#define EDID_MAX_DATA_SIZE 1024
#endif

// DS HAL function type definitions
typedef dsError_t (*dsGetHostEDIDFunc_t)(unsigned char *edid, int *length);

class dHostImpl : public hal::dHost::IPlatform {

    // delete copy constructor and assignment operator
    dHostImpl(const dHostImpl&) = delete;
    dHostImpl& operator=(const dHostImpl&) = delete;

public:
    dHostImpl()
    {
        DSLOG_INFO("Constructor");
        getInstance() = this; // Set static instance for callback access
        InitialiseHAL();
    }

    virtual ~dHostImpl()
    {
        DSLOG_INFO("Destructor");
        DeInitialiseHAL();
        getInstance() = nullptr; // Clear static instance
    }

    // Singleton getInstance method - following VideoPort/HDMIIn pattern
    static dHostImpl*& getInstance()
    {
        static dHostImpl* instance = nullptr;
        return instance;
    }

    void InitialiseHAL()
    {
        // Note: host_isInitialized should only be set in setAllCallbacks after callback registration
        // Don't set it here as it prevents callback registration condition from working

        if (!host_isPlatInitialized) {
            DSLOG_INFO("<dsHost>");
            dsError_t eError = dsHostInit();
            if (dsERR_NONE != eError) {
                DSLOG_ERR(" dsHostInit failed with error: %d", eError);
                return;
            }
            host_isPlatInitialized = 1;
            DSLOG_INFO(" dsHost HAL initialized successfully");
            
            // Load persistence values - following dsHost.cpp dsHostMgr_init pattern
            getPersistenceValue();
        }
    }

    void DeInitialiseHAL()
    {
        if (host_isPlatInitialized) {
            host_isPlatInitialized = 0;
            DSLOG_INFO(" dsHost HAL de-initialized successfully");
        }
    }

    uint32_t GetEDID(uint8_t edId[], const uint16_t edIdLength) override
    {
        uint32_t retCode = WPEFramework::Core::ERROR_GENERAL;
        DSLOG_INFO(" edIdLength=%u", edIdLength);

        // Use resolve function following dHdmiInImpl.h pattern
        typedef dsError_t (*dsGetHostEDIDFunc_t)(unsigned char *edid, int *length);
        dsGetHostEDIDFunc_t func = (dsGetHostEDIDFunc_t)resolve(RDK_DSHAL_NAME, "dsGetHostEDID");

        if (func != nullptr) {
            unsigned char edidBytes[EDID_MAX_DATA_SIZE];
            int length = 0;
            dsError_t eError = func(edidBytes, &length);
            if (eError == dsERR_NONE && length <= static_cast<int>(edIdLength)) {
                memcpy(edId, edidBytes, length);
                retCode = WPEFramework::Core::ERROR_NONE;
                DSLOG_INFO(" SUCCESS - copied %d bytes", length);
            } else if (eError == dsERR_NONE && length > static_cast<int>(edIdLength)) {
                DSLOG_ERR(" Buffer too small - required %d bytes, provided %u", length, edIdLength);
                retCode = WPEFramework::Core::ERROR_BAD_REQUEST;
            } else {
                DSLOG_ERR(" dsGetHostEDID failed with error: %d", eError);
            }
        } else {
            retCode = WPEFramework::Core::ERROR_UNAVAILABLE;
            DSLOG_ERR(" Function not available");
        }

        return retCode;
    }

    uint32_t GetMS12ConfigType(string &ms12Config) override
    {
        uint32_t retCode = WPEFramework::Core::ERROR_GENERAL;
        
        // Following dsHost.cpp pattern
        try {
            ms12Config = device::HostPersistence::getInstance().getDefaultProperty("MS12.Config.Type");
            DSLOG_INFO(" SUCCESS - ms12Config='%s'", ms12Config.c_str());
            retCode = WPEFramework::Core::ERROR_NONE;
        } catch (const std::exception& e) {
            DSLOG_WARN(" Failed to retrieve config from default persistence: %s", e.what());
            ms12Config = "CONFIG_NONE";
            retCode = WPEFramework::Core::ERROR_NONE;
        } catch (...) {
            DSLOG_WARN(" Unknown error retrieving config from default persistence");
            ms12Config = "CONFIG_NONE";
            retCode = WPEFramework::Core::ERROR_NONE;
        }
        
        return retCode;
    }

    void setAllCallbacks(const CallbackBundle& bundle) override
    {
        ENTRY_LOG;
        if (host_isPlatInitialized && !host_isInitialized) {
            host_isInitialized = 1;
            DSLOG_INFO("Host platform callback Initialization done");
        }
        EXIT_LOG;
    }

    void getPersistenceValue() override
    {
        ENTRY_LOG;
        EXIT_LOG;
    }

private:
    // Dynamic loading helper - following dHdmiInImpl.h pattern
    static void* resolve(const std::string& libName, const std::string& symbolName) {
        void* handle = dlopen(libName.c_str(), RTLD_LAZY);
        if (!handle) {
            std::cerr << "dlopen failed for " << libName << ": " << dlerror() << std::endl;
            return nullptr;
        }
        void* symbol = dlsym(handle, symbolName.c_str());
        if (!symbol) {
            std::cerr << "dlsym failed for " << symbolName << ": " << dlerror() << std::endl;
        }
        dlclose(handle);
        return symbol;
    }
};