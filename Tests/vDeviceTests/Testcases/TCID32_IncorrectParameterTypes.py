"""
/**
 * @file TCID32_IncorrectParameterTypes.py
 * @brief Verifies strict JSON parameter type validation.
 *
 * @testcase TCID32_IncorrectParameterTypes
 * @details Sends valid parameter names with invalid JSON value types, verifies
 *          rejection, restores writable settings, and checks plugin health.
 */
"""

import os
import time

import AVInput_Curl as AVInputApis
from AVInput_Helpers import (
    parse_allm_support,
    parse_edid_version,
    parse_number_of_inputs,
    parse_vrr_support,
    result_rejected,
)
from utils import log_error, log_info, log_success, log_warning, send_curl_command

PORT = 0


def run_test():
    start_time = time.perf_counter()
    original_version = parse_edid_version(
        send_curl_command(AVInputApis.get_edid_version(PORT))
    )
    original_allm = parse_allm_support(
        send_curl_command(AVInputApis.get_edid2_allm_support(PORT))
    )
    original_vrr = parse_vrr_support(
        send_curl_command(AVInputApis.get_vrr_support(PORT))
    )

    cases = [
        ("getHdmiVersion portId=null", "getHdmiVersion", {"portId": None}),
        ("getHdmiVersion portId=true", "getHdmiVersion", {"portId": True}),
        ("getHdmiVersion portId=float", "getHdmiVersion", {"portId": 0.5}),
        ("getHdmiVersion portId=array", "getHdmiVersion", {"portId": []}),
        ("getHdmiVersion portId=object", "getHdmiVersion", {"portId": {}}),
        ("getHdmiVersion portId=empty string", "getHdmiVersion", {"portId": ""}),
        (
            "startInput portId=array",
            "startInput",
            {
                "portId": [],
                "typeOfInput": AVInputApis.TYPE_HDMI,
                "requestAudioMix": False,
                "plane": 0,
                "topMost": False,
            },
        ),
        (
            "setEdid2AllmSupport allmSupport=string",
            "setEdid2AllmSupport",
            {"portId": PORT, "allmSupport": "true"},
        ),
        (
            "setVRRSupport vrrSupport=integer",
            "setVRRSupport",
            {"portId": PORT, "vrrSupport": 1},
        ),
        (
            "setEdidVersion edidVersion=integer",
            "setEdidVersion",
            {"portId": PORT, "edidVersion": 2},
        ),
        (
            "setMixerLevels primaryVolume=string",
            "setMixerLevels",
            {"primaryVolume": "50", "inputVolume": 50},
        ),
        (
            "setVideoRectangle width=array",
            "setVideoRectangle",
            {
                "x": 0,
                "y": 0,
                "w": [],
                "h": 360,
                "typeOfInput": AVInputApis.TYPE_HDMI,
            },
        ),
    ]

    try:
        for label, method, params in cases:
            log_info(f"Sending request with incorrect parameter type: {label}")
            response = send_curl_command(AVInputApis.raw_api_request(method, params))
            log_warning(f"Response: {response}")
            if not result_rejected(response):
                log_error(
                    f"TCID32_IncorrectParameterTypes Failed ({label} was not rejected)"
                )
                return False

        health = send_curl_command(AVInputApis.number_of_inputs)
        if parse_number_of_inputs(health) is None:
            log_error(
                "TCID32_IncorrectParameterTypes Failed "
                "(numberOfInputs unavailable after negative requests)"
            )
            return False
    finally:
        send_curl_command(AVInputApis.stop_input(AVInputApis.TYPE_HDMI))
        if isinstance(original_allm, bool):
            send_curl_command(
                AVInputApis.set_edid2_allm_support(PORT, original_allm)
            )
        if isinstance(original_vrr, bool):
            send_curl_command(AVInputApis.set_vrr_support(PORT, original_vrr))
        if original_version in (
            AVInputApis.EDID_VERSION_14,
            AVInputApis.EDID_VERSION_20,
        ):
            send_curl_command(
                AVInputApis.set_edid_version(PORT, original_version)
            )

    elapsed_time = time.perf_counter() - start_time
    message = "TCID32_IncorrectParameterTypes Passed"
    if os.environ.get("AVINPUT_TIMING_ENABLED"):
        log_success(f"{message} time consumed: {elapsed_time:.3f}s")
    else:
        log_success(message)
    return True
