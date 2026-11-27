"""
/**
 * @file TCID34_InvalidLifecycleSequences.py
 * @brief Verifies recovery across invalid HDMI input lifecycle operations.
 *
 * @testcase TCID34_InvalidLifecycleSequences
 * @details Uses numberOfInputs to derive an invalid port, rejects invalid starts
 *          before and during a valid session, and verifies stop idempotency and
 *          valid recovery.
 */
"""

import os
import time

import AVInput_Curl as AVInputApis
from AVInput_Helpers import (
    parse_number_of_inputs,
    result_rejected,
    result_success,
)
from utils import log_error, log_info, log_success, log_warning, send_curl_command

VALID_PORT = 0


def _expect_success(label, command):
    response = send_curl_command(command)
    log_warning(f"{label} response: {response}")
    if not result_success(response):
        log_error(f"TCID34_InvalidLifecycleSequences Failed ({label} failed)")
        return False
    return True


def _expect_rejected(label, command):
    response = send_curl_command(command)
    log_warning(f"{label} response: {response}")
    if not result_rejected(response):
        log_error(f"TCID34_InvalidLifecycleSequences Failed ({label} was not rejected)")
        return False
    return True


def run_test():
    start_time = time.perf_counter()
    count = parse_number_of_inputs(send_curl_command(AVInputApis.number_of_inputs))
    if count is None or count <= VALID_PORT:
        log_error(
            "TCID34_InvalidLifecycleSequences Failed "
            "(could not derive an invalid port)"
        )
        return False
    invalid_port = count

    try:
        log_info("Establishing stopped baseline")
        if not _expect_success(
            "stopInput before start",
            AVInputApis.stop_input(AVInputApis.TYPE_HDMI),
        ):
            return False

        if not _expect_rejected(
            f"startInput invalid port {invalid_port} before valid start",
            AVInputApis.start_input(invalid_port),
        ):
            return False

        if not _expect_success(
            f"startInput valid port {VALID_PORT}",
            AVInputApis.start_input(VALID_PORT),
        ):
            return False

        if not _expect_rejected(
            f"startInput invalid port {invalid_port} during active session",
            AVInputApis.start_input(invalid_port),
        ):
            return False

        if not _expect_success(
            "stopInput after rejected switch",
            AVInputApis.stop_input(AVInputApis.TYPE_HDMI),
        ):
            return False

        if not _expect_success(
            "redundant stopInput",
            AVInputApis.stop_input(AVInputApis.TYPE_HDMI),
        ):
            return False

        if not _expect_success(
            "recovery startInput",
            AVInputApis.start_input(VALID_PORT),
        ):
            return False
    finally:
        send_curl_command(AVInputApis.stop_input(AVInputApis.TYPE_HDMI))

    elapsed_time = time.perf_counter() - start_time
    message = "TCID34_InvalidLifecycleSequences Passed"
    if os.environ.get("AVINPUT_TIMING_ENABLED"):
        log_success(f"{message} time consumed: {elapsed_time:.3f}s")
    else:
        log_success(message)
    return True
