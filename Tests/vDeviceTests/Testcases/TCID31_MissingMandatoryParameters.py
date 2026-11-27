"""
/**
 * @file TCID31_MissingMandatoryParameters.py
 * @brief Verifies that AVInput rejects requests with missing required fields.
 *
 * @testcase TCID31_MissingMandatoryParameters
 * @details Sends partial requests for lifecycle and setter APIs, verifies strict
 *          rejection, then confirms that the plugin remains responsive.
 */
"""

import os
import time

import AVInput_Curl as AVInputApis
from AVInput_Helpers import parse_number_of_inputs, result_rejected
from utils import log_error, log_info, log_success, log_warning, send_curl_command


def run_test():
    start_time = time.perf_counter()
    cases = [
        (
            "startInput missing portId",
            "startInput",
            {
                "typeOfInput": AVInputApis.TYPE_HDMI,
                "requestAudioMix": False,
                "plane": 0,
                "topMost": False,
            },
        ),
        (
            "startInput missing typeOfInput",
            "startInput",
            {"portId": 0, "requestAudioMix": False, "plane": 0, "topMost": False},
        ),
        ("setEdidVersion missing edidVersion", "setEdidVersion", {"portId": 0}),
        (
            "setEdid2AllmSupport missing allmSupport",
            "setEdid2AllmSupport",
            {"portId": 0},
        ),
        ("setVRRSupport missing vrrSupport", "setVRRSupport", {"portId": 0}),
        (
            "setVideoRectangle missing width",
            "setVideoRectangle",
            {"x": 0, "y": 0, "h": 360, "typeOfInput": AVInputApis.TYPE_HDMI},
        ),
        (
            "setVideoRectangle missing height",
            "setVideoRectangle",
            {"x": 0, "y": 0, "w": 640, "typeOfInput": AVInputApis.TYPE_HDMI},
        ),
        ("setMixerLevels missing primaryVolume", "setMixerLevels", {"inputVolume": 50}),
        ("setMixerLevels missing inputVolume", "setMixerLevels", {"primaryVolume": 50}),
    ]

    try:
        for label, method, params in cases:
            log_info(f"Sending request with missing parameter: {label}")
            response = send_curl_command(AVInputApis.raw_api_request(method, params))
            log_warning(f"Response: {response}")
            if not result_rejected(response):
                log_error(
                    "TCID31_MissingMandatoryParameters Failed "
                    f"({label} was not explicitly rejected)"
                )
                return False

        health = send_curl_command(AVInputApis.number_of_inputs)
        if parse_number_of_inputs(health) is None:
            log_error(
                "TCID31_MissingMandatoryParameters Failed "
                "(numberOfInputs unavailable after negative requests)"
            )
            return False
    finally:
        send_curl_command(AVInputApis.stop_input(AVInputApis.TYPE_HDMI))

    elapsed_time = time.perf_counter() - start_time
    message = "TCID31_MissingMandatoryParameters Passed"
    if os.environ.get("AVINPUT_TIMING_ENABLED"):
        log_success(f"{message} time consumed: {elapsed_time:.3f}s")
    else:
        log_success(message)
    return True
