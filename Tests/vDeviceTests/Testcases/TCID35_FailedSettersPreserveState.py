"""
/**
 * @file TCID35_FailedSettersPreserveState.py
 * @brief Verifies that rejected setters do not modify HDMI input settings.
 *
 * @testcase TCID35_FailedSettersPreserveState
 * @details Captures EDID version, ALLM, and VRR state; sends invalid setters;
 *          verifies strict rejection and confirms each value is unchanged.
 */
"""

import os
import time

import AVInput_Curl as AVInputApis
from AVInput_Helpers import (
    parse_allm_support,
    parse_edid_version,
    parse_vrr_support,
    result_rejected,
)
from utils import log_error, log_info, log_success, log_warning, send_curl_command

PORT = 0


def _verify_unchanged(label, method, params, getter, parser, expected):
    log_info(f"Sending invalid setter: {label}")
    response = send_curl_command(AVInputApis.raw_api_request(method, params))
    log_warning(f"{label} response: {response}")
    if not result_rejected(response):
        log_error(
            f"TCID35_FailedSettersPreserveState Failed ({label} was not rejected)"
        )
        return False

    current = parser(send_curl_command(getter))
    if current != expected:
        log_error(
            "TCID35_FailedSettersPreserveState Failed "
            f"({label} changed state from {expected!r} to {current!r})"
        )
        return False
    log_success(
        f"Previous state preserved after {label}: value remains {expected!r}"
    )
    return True


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
    log_info(
        "Captured previous state: "
        f"edidVersion={original_version!r}, "
        f"allmSupport={original_allm!r}, "
        f"vrrSupport={original_vrr!r}"
    )

    if original_version not in (
        AVInputApis.EDID_VERSION_14,
        AVInputApis.EDID_VERSION_20,
    ):
        log_error(
            "TCID35_FailedSettersPreserveState Failed "
            "(could not capture EDID version)"
        )
        return False
    if not isinstance(original_allm, bool) or not isinstance(original_vrr, bool):
        log_error(
            "TCID35_FailedSettersPreserveState Failed "
            "(could not capture ALLM/VRR state)"
        )
        return False

    try:
        checks = [
            (
                "invalid EDID version",
                "setEdidVersion",
                {"portId": PORT, "edidVersion": "HDMI9.9"},
                AVInputApis.get_edid_version(PORT),
                parse_edid_version,
                original_version,
            ),
            (
                "string ALLM support",
                "setEdid2AllmSupport",
                {"portId": PORT, "allmSupport": "true"},
                AVInputApis.get_edid2_allm_support(PORT),
                parse_allm_support,
                original_allm,
            ),
            (
                "string VRR support",
                "setVRRSupport",
                {"portId": PORT, "vrrSupport": "true"},
                AVInputApis.get_vrr_support(PORT),
                parse_vrr_support,
                original_vrr,
            ),
        ]

        for check in checks:
            if not _verify_unchanged(*check):
                return False
    finally:
        send_curl_command(
            AVInputApis.set_edid2_allm_support(PORT, original_allm)
        )
        send_curl_command(AVInputApis.set_vrr_support(PORT, original_vrr))
        send_curl_command(
            AVInputApis.set_edid_version(PORT, original_version)
        )

    elapsed_time = time.perf_counter() - start_time
    message = "TCID35_FailedSettersPreserveState Passed"
    if os.environ.get("AVINPUT_TIMING_ENABLED"):
        log_success(f"{message} time consumed: {elapsed_time:.3f}s")
    else:
        log_success(message)
    return True
