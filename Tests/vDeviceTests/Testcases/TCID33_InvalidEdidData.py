"""
/**
 * @file TCID33_InvalidEdidData.py
 * @brief Verifies rejection of malformed EDID payloads.
 *
 * @testcase TCID33_InvalidEdidData
 * @details Writes malformed Base64, invalid-length, and invalid-content EDID
 *          values. Each request must be rejected and the original EDID must
 *          remain unchanged.
 */
"""

import base64
import os
import time

import AVInput_Curl as AVInputApis
from AVInput_Helpers import parse_edid, result_rejected
from utils import log_error, log_info, log_success, log_warning, send_curl_command

PORT = 0


def run_test():
    start_time = time.perf_counter()
    original_edid = parse_edid(send_curl_command(AVInputApis.read_edid(PORT)))
    if not original_edid:
        log_error("TCID33_InvalidEdidData Failed (could not capture original EDID)")
        return False

    cases = [
        ("empty EDID", ""),
        ("invalid Base64", "not-valid-base64!"),
        ("127 decoded bytes", base64.b64encode(bytes(127)).decode("ascii")),
        ("129 decoded bytes", base64.b64encode(bytes(129)).decode("ascii")),
        (
            "128 bytes with invalid EDID header and checksum",
            base64.b64encode(bytes([1]) * 128).decode("ascii"),
        ),
        ("null EDID", None),
        ("array EDID", []),
    ]

    try:
        for label, message in cases:
            log_info(f"Writing malformed EDID: {label}")
            response = send_curl_command(
                AVInputApis.raw_api_request(
                    "writeEDID", {"portId": PORT, "message": message}
                )
            )
            log_warning(f"Response: {response}")
            if not result_rejected(response):
                log_error(
                    f"TCID33_InvalidEdidData Failed ({label} was not rejected)"
                )
                return False

            current_edid = parse_edid(
                send_curl_command(AVInputApis.read_edid(PORT))
            )
            if current_edid != original_edid:
                log_error(
                    "TCID33_InvalidEdidData Failed "
                    f"({label} changed the stored EDID)"
                )
                return False
    finally:
        send_curl_command(AVInputApis.write_edid(PORT, original_edid))

    elapsed_time = time.perf_counter() - start_time
    message = "TCID33_InvalidEdidData Passed"
    if os.environ.get("AVINPUT_TIMING_ENABLED"):
        log_success(f"{message} time consumed: {elapsed_time:.3f}s")
    else:
        log_success(message)
    return True
