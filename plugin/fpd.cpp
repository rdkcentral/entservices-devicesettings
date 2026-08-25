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

#include <functional>
#include <unistd.h>

#include <core/IAction.h>
#include <core/Time.h>
#include <core/WorkerPool.h>

#include "secure_wrapper.h"
#include "fpd.h"

FPD::FPD(INotification& parent, std::shared_ptr<IPlatform> platform)
    : _platform(std::move(platform))
    , _parent(parent)
{
    Platform_init();
}

void FPD::Platform_init()
{
    // Initialize FPD platform
    DSLOG_INFO("FPD Init");
}

//Depricated
uint32_t FPD::SetFPDTime(const FPDTimeFormat timeFormat, const uint32_t minutes, const uint32_t seconds) {
    DSLOG_INFO("timeFormat=%d, minutes=%u, seconds=%u", timeFormat, minutes, seconds);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().SetFPDTime(timeFormat, minutes, seconds);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - platform call completed successfully");
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
}

uint32_t FPD::SetFPDScroll(const uint32_t scrollHoldDuration, const uint32_t nHorizontalScrollIterations, const uint32_t nVerticalScrollIterations) {
    DSLOG_INFO("scrollHoldDuration=%u, horizontal=%u, vertical=%u", scrollHoldDuration, nHorizontalScrollIterations, nVerticalScrollIterations);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().SetFPDScroll(scrollHoldDuration, nHorizontalScrollIterations, nVerticalScrollIterations);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - platform call completed successfully");
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
}

uint32_t FPD::SetFPDTextBrightness(const FPDTextDisplay textDisplay, const uint32_t brightNess) {
    DSLOG_INFO("textDisplay=%d, brightNess=%u", textDisplay, brightNess);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().SetFPDTextBrightness(textDisplay, brightNess);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - platform call completed successfully");
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
}

uint32_t FPD::GetFPDTextBrightness(const FPDTextDisplay textDisplay, uint32_t &brightNess) {
    DSLOG_INFO("textDisplay=%d", textDisplay);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().GetFPDTextBrightness(textDisplay, brightNess);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - textDisplay=%d, brightNess=%u", textDisplay, brightNess);
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
}

uint32_t FPD::EnableFPDClockDisplay(const bool enable) {
    DSLOG_INFO("enable=%s", enable ? "true" : "false");
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().EnableFPDClockDisplay(enable);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - platform call completed successfully");
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
}

uint32_t FPD::GetFPDTimeFormat(FPDTimeFormat &fpdTimeFormat) {
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().GetFPDTimeFormat(fpdTimeFormat);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - fpdTimeFormat=%d", fpdTimeFormat);
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
}

uint32_t FPD::SetFPDTimeFormat(const FPDTimeFormat fpdTimeFormat) {
    DSLOG_INFO("fpdTimeFormat=%d", fpdTimeFormat);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().SetFPDTimeFormat(fpdTimeFormat);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - platform call completed successfully");
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
}
//Depricated

uint32_t FPD::SetFPDBlink(const FPDIndicator indicator, const uint32_t blinkDuration, const uint32_t blinkIterations) {

    DSLOG_INFO("indicator=%d, blinkDuration=%u, blinkIterations:%u", indicator, blinkDuration, blinkIterations);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().SetFPDBlink(indicator, blinkDuration, blinkIterations);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - platform call completed successfully");
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }

    return result;
}

uint32_t FPD::GetFPDBrightness(const FPDIndicator indicator, uint32_t &brightNess, const bool persist) {

    DSLOG_INFO("indicator=%d, persist=%s", indicator, persist ? "true" : "false");
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().GetFPDBrightness(indicator, brightNess, persist);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - indicator=%d, brightNess=%d", indicator, brightNess);
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }

    return result;
}

uint32_t FPD::SetFPDBrightness(const FPDIndicator indicator, const uint32_t brightNess, const bool persist) {

    DSLOG_INFO("indicator=%d, brightNess=%u, persist=%s", indicator, brightNess, persist ? "true" : "false");
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().SetFPDBrightness(indicator, brightNess, persist);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - platform call completed successfully");
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }

    return result;
}

uint32_t FPD::GetFPDState(const FPDIndicator indicator, FPDState &state) {

    DSLOG_INFO("indicator=%d", indicator);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().GetFPDState(indicator, state);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - indicator=%d, state=%d", indicator, state);
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }

    return result;
}

uint32_t FPD::SetFPDState(const FPDIndicator indicator, const FPDState state) {

    DSLOG_INFO("indicator=%d, state=%d", indicator, state);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().SetFPDState(indicator, state);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - platform call completed successfully");
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }

    return result;
}

uint32_t FPD::GetFPDColor(const FPDIndicator indicator, uint32_t &color) {

    DSLOG_INFO("indicator=%d", indicator);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().GetFPDColor(indicator, color);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - indicator=%d, colour=%d", indicator, color);
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
}

uint32_t FPD::SetFPDColor(const FPDIndicator indicator, const uint32_t color) {

    DSLOG_INFO("indicator=%d, colour=%d", indicator, color);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().SetFPDColor(indicator, color);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - indicator=%d, colour=%d", indicator, color);
    } else {
        DSLOG_ERR("FAILED - indicator=%d, colour=%d, result=%u", indicator, color, result);
    }

    return result;
}

uint32_t FPD::SetFPDMode(const FPDMode fpdMode) {
    DSLOG_INFO("fpdMode=%d", fpdMode);
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    if (_platform) {
        result = this->platform().SetFPDMode(fpdMode);
    }
    if (result == WPEFramework::Core::ERROR_NONE) {
        DSLOG_INFO("SUCCESS - platform call completed successfully");
    } else {
        DSLOG_ERR("FAILED - result=%u", result);
    }
    return result;
}
