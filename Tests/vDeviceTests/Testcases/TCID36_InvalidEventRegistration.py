"""
/**
 * @file TCID36_InvalidEventRegistration.py
 * @brief Verifies rejection of malformed AVInput event registrations.
 *
 * @testcase TCID36_InvalidEventRegistration
 * @details Attempts unknown and empty event names plus a null listener ID. Each
 *          registration must be rejected, and a subsequent valid registration
 *          must still succeed.
 */
"""

import os
import time

import AVInput_Curl as AVInputApis
from utils import JsonRpcEventListener, log_error, log_info, log_success

EVENT_TIMEOUT = float(
    os.environ.get("AVINPUT_NEGATIVE_EVENT_TIMEOUT", "12")
)


def run_test():
    start_time = time.perf_counter()
    invalid_registrations = [
        ("unknown event", "__invalidEvent__", "ID_TCID36_unknown"),
        ("empty event name", "", "ID_TCID36_empty"),
        ("null listener ID", "onInputStatusChanged", None),
    ]

    for label, event_name, listener_id in invalid_registrations:
        log_info(f"Attempting invalid event registration: {label}")
        listener = JsonRpcEventListener(
            AVInputApis.CALLSIGN,
            event_name,
            listener_id,
            timeout=EVENT_TIMEOUT,
        )
        try:
            if listener.connect():
                log_error(
                    "TCID36_InvalidEventRegistration Failed "
                    f"({label} was accepted)"
                )
                return False
            log_success(f"Invalid event registration rejected: {label}")
        finally:
            listener.close()

    log_info("Verifying valid event registration still succeeds")
    recovery_listener = JsonRpcEventListener(
        AVInputApis.CALLSIGN,
        "onInputStatusChanged",
        "ID_TCID36_recovery",
        timeout=EVENT_TIMEOUT,
    )
    try:
        if not recovery_listener.connect():
            log_error(
                "TCID36_InvalidEventRegistration Failed "
                "(valid recovery registration rejected)"
            )
            return False
        log_success("Valid event registration succeeded after invalid attempts")
    finally:
        recovery_listener.close()

    elapsed_time = time.perf_counter() - start_time
    message = "TCID36_InvalidEventRegistration Passed"
    if os.environ.get("AVINPUT_TIMING_ENABLED"):
        log_success(f"{message} time consumed: {elapsed_time:.3f}s")
    else:
        log_success(message)
    return True
