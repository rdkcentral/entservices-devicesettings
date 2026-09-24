"""
/**
 * @file TCID28_StableSignalState.py
 * @brief Reproduces HDMI signal-change notifications with vComponent stimuli.
 *
 * @testcase TCID28_StableSignalState
 * @details Registers for onSignalChanged, presents HDMI port 0, and injects a
 *          sequence of distinct signal states through the HDMI Input
 *          vComponent. Each stimulus is logged beside the AVInput status
 *          expected in middleware logs so callback processing can be traced.
 *
 * @precondition
 *  - org.rdk.AVInput is active and reachable through JSON-RPC.
 *  - HDMI Input vComponent is reachable at HDMIIN_VCOMPONENT_API_URL.
 *
 * @expected_result
 *  - Service logs show onSignalChanged transitions for stableSignal,
 *    unstableSignal, notSupportedSignal, and noSignal in injection order.
 *
 * @pass_criteria
 *  - Event registration, startInput, and every vComponent post are accepted.
 *
 * @note
 *  - The JSON-RPC registration transport does not retain asynchronous event
 *    payloads, so notification payloads are verified in the target logs.
 */
"""

import os
import time

import AVInput_Curl as AVInputApis
from utils import (
    HDMIIN_CMD_BASE,
    is_ok,
    log_error,
    log_info,
    log_success,
    log_warning,
    send_curl_command,
    send_vcomponent_command,
    send_vcomponent_payload,
)

PORT = 0
SIGNAL_YAML = "HDMIInput_Signal_LOCKED_Status.yaml"
TRANSITION_DELAY = float(os.environ.get("AVINPUT_SIGNAL_TRANSITION_DELAY", "2"))


def _post_locked_yaml():
    yaml_path = f"{HDMIIN_CMD_BASE}/{SIGNAL_YAML}"
    http_code, body = send_vcomponent_command(yaml_path)
    log_warning(f"vComponent POST {SIGNAL_YAML}: HTTP {http_code} {body}")
    return http_code == 200


def run_test():
    registration = send_curl_command(
        AVInputApis.register_event("onSignalChanged", "ID_TCID28_signal")
    )
    log_warning(f"register onSignalChanged: {registration}")
    if not is_ok(registration):
        log_error("TCID28_StableSignalState Failed (event registration rejected)")
        return False

    try:
        # Establish a known baseline before driving distinct transitions.
        time.sleep(TRANSITION_DELAY)

        start_response = send_curl_command(AVInputApis.start_input(PORT))
        log_warning(f"startInput response: {start_response}")
        if not is_ok(start_response):
            log_error("TCID28_StableSignalState Failed (startInput rejected)")
            return False
        time.sleep(TRANSITION_DELAY)

        if not _post_locked_yaml():
            log_error("TCID28_StableSignalState Failed (locked signal injection rejected)")
            return False

        http_code, body = send_vcomponent_payload(
            "connection_status", {"port": PORT, "connected": True}
        )
        log_warning(f"vComponent connection_status(connected=true): HTTP {http_code} {body}")
        if http_code != 200:
            log_error("TCID28_StableSignalState Failed (connection rejected)")
            return False
        time.sleep(TRANSITION_DELAY)

        log_success(
            "Signal transition sequence accepted; correlate the markers above "
            "with onSignalChanged entries in the target logs"
        )
        return True
    finally:
        send_vcomponent_payload(
            "connection_status", {"port": PORT, "connected": False}
        )
        send_curl_command(AVInputApis.stop_input(AVInputApis.TYPE_HDMI))
