"""
/**
 * @file TCID37_RejectedRequestsDoNotEmitEvents.py
 * @brief Verifies rejected lifecycle requests emit no input-status event.
 *
 * @testcase TCID37_RejectedRequestsDoNotEmitEvents
 * @details Listens for onInputStatusChanged while sending an out-of-range start
 *          and a start request missing typeOfInput. Both requests must be
 *          rejected without an event. A valid start must subsequently emit the
 *          expected notification.
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
from utils import (
    JsonRpcEventListener,
    log_error,
    log_info,
    log_success,
    log_warning,
    send_curl_command,
)

PORT = 0
EVENT_TIMEOUT = float(
    os.environ.get("AVINPUT_NEGATIVE_EVENT_TIMEOUT", "12")
)


def _expect_no_event(listener, label, predicate):
    notification = listener.wait_for_event(predicate, timeout=EVENT_TIMEOUT)
    if notification is not None:
        log_error(
            "TCID37_RejectedRequestsDoNotEmitEvents Failed "
            f"({label} emitted notification {notification})"
        )
        return False
    log_success(
        f"No notification observed for {label} during "
        f"{EVENT_TIMEOUT:g}-second window"
    )
    return True


def run_test():
    start_time = time.perf_counter()
    count = parse_number_of_inputs(
        send_curl_command(AVInputApis.number_of_inputs)
    )
    if count is None or count <= PORT:
        log_error(
            "TCID37_RejectedRequestsDoNotEmitEvents Failed "
            "(could not derive an invalid port)"
        )
        return False
    invalid_port = count

    listener = JsonRpcEventListener(
        AVInputApis.CALLSIGN,
        "onInputStatusChanged",
        "ID_TCID37_input_status",
        timeout=EVENT_TIMEOUT,
    )
    if not listener.connect():
        log_error(
            "TCID37_RejectedRequestsDoNotEmitEvents Failed "
            "(event registration rejected)"
        )
        return False

    try:
        send_curl_command(AVInputApis.stop_input(AVInputApis.TYPE_HDMI))

        log_info(f"Sending startInput with invalid port {invalid_port}")
        invalid_port_response = send_curl_command(
            AVInputApis.start_input(invalid_port)
        )
        log_warning(f"Invalid-port startInput response: {invalid_port_response}")
        if not result_rejected(invalid_port_response):
            log_error(
                "TCID37_RejectedRequestsDoNotEmitEvents Failed "
                "(invalid-port start was not rejected)"
            )
            return False
        if not _expect_no_event(
            listener,
            "invalid-port startInput",
            lambda params: str(params.get("id")) == str(invalid_port),
        ):
            return False

        log_info("Sending startInput without typeOfInput")
        missing_type_response = send_curl_command(
            AVInputApis.raw_api_request(
                "startInput",
                {
                    "portId": PORT,
                    "requestAudioMix": False,
                    "plane": 0,
                    "topMost": False,
                },
            )
        )
        log_warning(
            f"Missing-type startInput response: {missing_type_response}"
        )
        if not result_rejected(missing_type_response):
            log_error(
                "TCID37_RejectedRequestsDoNotEmitEvents Failed "
                "(missing-type start was not rejected)"
            )
            return False
        if not _expect_no_event(
            listener,
            "startInput missing typeOfInput",
            lambda params: (
                str(params.get("id")) == str(PORT)
                and params.get("status") == "started"
            ),
        ):
            return False

        log_info("Verifying a valid start still emits onInputStatusChanged")
        valid_response = send_curl_command(AVInputApis.start_input(PORT))
        log_warning(f"Valid startInput response: {valid_response}")
        if not result_success(valid_response):
            log_error(
                "TCID37_RejectedRequestsDoNotEmitEvents Failed "
                "(valid recovery start rejected)"
            )
            return False
        notification = listener.wait_for_event(
            lambda params: (
                str(params.get("id")) == str(PORT)
                and params.get("status") == "started"
            ),
            timeout=EVENT_TIMEOUT,
        )
        if notification is None:
            log_error(
                "TCID37_RejectedRequestsDoNotEmitEvents Failed "
                "(valid recovery start emitted no notification)"
            )
            return False
        log_success(
            f"Valid recovery start emitted expected notification: {notification}"
        )
    finally:
        send_curl_command(AVInputApis.stop_input(AVInputApis.TYPE_HDMI))
        listener.close()

    elapsed_time = time.perf_counter() - start_time
    message = "TCID37_RejectedRequestsDoNotEmitEvents Passed"
    if os.environ.get("AVINPUT_TIMING_ENABLED"):
        log_success(f"{message} time consumed: {elapsed_time:.3f}s")
    else:
        log_success(message)
    return True
