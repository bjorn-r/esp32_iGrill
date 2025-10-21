#!/usr/bin/env python3
"""
Phase 1 Test - Core Component Structure
Tests for igrill_client component files
"""

import os
import sys
import re

# Color codes for output
GREEN = '\033[92m'
RED = '\033[91m'
YELLOW = '\033[93m'
RESET = '\033[0m'
BOLD = '\033[1m'

def print_success(msg):
    print(f"{GREEN}✓{RESET} {msg}")

def print_error(msg):
    print(f"{RED}✗{RESET} {msg}")

def print_info(msg):
    print(f"{YELLOW}ℹ{RESET} {msg}")

def print_header(msg):
    print(f"\n{BOLD}=== {msg} ==={RESET}")

# Base paths
BASE_DIR = "/home/user/esp32_iGrill/esphome-igrill"
COMPONENT_DIR = os.path.join(BASE_DIR, "components", "igrill_client")

# File paths
CONST_H = os.path.join(COMPONENT_DIR, "igrill_const.h")
CLIENT_H = os.path.join(COMPONENT_DIR, "igrill_client.h")
INIT_PY = os.path.join(COMPONENT_DIR, "__init__.py")

def test_file_existence():
    """Test P1.1, P1.2, P1.3: Check if all required files exist"""
    print_header("Testing File Existence")

    all_exist = True

    # Test igrill_const.h
    if os.path.exists(CONST_H):
        print_success(f"igrill_const.h exists at {CONST_H}")
    else:
        print_error(f"igrill_const.h NOT FOUND at {CONST_H}")
        all_exist = False

    # Test igrill_client.h
    if os.path.exists(CLIENT_H):
        print_success(f"igrill_client.h exists at {CLIENT_H}")
    else:
        print_error(f"igrill_client.h NOT FOUND at {CLIENT_H}")
        all_exist = False

    # Test __init__.py
    if os.path.exists(INIT_PY):
        print_success(f"__init__.py exists at {INIT_PY}")
    else:
        print_error(f"__init__.py NOT FOUND at {INIT_PY}")
        all_exist = False

    return all_exist

def test_const_h_content():
    """Test P1.1: Validate igrill_const.h content"""
    print_header("Testing igrill_const.h Content")

    if not os.path.exists(CONST_H):
        print_error("Cannot test content - file doesn't exist")
        return False

    with open(CONST_H, 'r') as f:
        content = f.read()

    checks_passed = True

    # Check for required UUIDs
    required_uuids = [
        'IGRILL_SERVICE_UUID',
        'PROBE1_UUID',
        'PROBE2_UUID',
        'PROBE3_UUID',
        'PROBE4_UUID',
        'BATTERY_UUID',
        'PROPANE_UUID',
        'APP_CHALLENGE_UUID',
        'DEVICE_CHALLENGE_UUID',
        'DEVICE_RESPONSE_UUID',
    ]

    for uuid in required_uuids:
        if uuid in content:
            print_success(f"Found {uuid}")
        else:
            print_error(f"Missing {uuid}")
            checks_passed = False

    # Check for namespace
    if 'namespace esphome' in content and 'namespace igrill_client' in content:
        print_success("Correct namespace structure")
    else:
        print_error("Missing or incorrect namespace")
        checks_passed = False

    # Check for constants
    if 'IGRILL_MAX_PROBES' in content:
        print_success("Found IGRILL_MAX_PROBES constant")
    else:
        print_error("Missing IGRILL_MAX_PROBES constant")
        checks_passed = False

    return checks_passed

def test_client_h_content():
    """Test P1.2: Validate igrill_client.h content"""
    print_header("Testing igrill_client.h Content")

    if not os.path.exists(CLIENT_H):
        print_error("Cannot test content - file doesn't exist")
        return False

    with open(CLIENT_H, 'r') as f:
        content = f.read()

    checks_passed = True

    # Check for class definition
    if 'class IGrillClient' in content:
        print_success("Found IGrillClient class definition")
    else:
        print_error("Missing IGrillClient class definition")
        checks_passed = False

    # Check for inheritance
    if 'BLEClientNode' in content and 'PollingComponent' in content:
        print_success("Correct inheritance from BLEClientNode and PollingComponent")
    else:
        print_error("Missing or incorrect inheritance")
        checks_passed = False

    # Check for required methods
    required_methods = [
        'setup',
        'update',
        'gattc_event_handler',
        'dump_config',
    ]

    for method in required_methods:
        if method in content:
            print_success(f"Found {method} method")
        else:
            print_error(f"Missing {method} method")
            checks_passed = False

    # Check for sensor setters
    sensor_setters = [
        'set_probe1_sensor',
        'set_probe2_sensor',
        'set_probe3_sensor',
        'set_probe4_sensor',
        'set_battery_sensor',
        'set_propane_sensor',
    ]

    for setter in sensor_setters:
        if setter in content:
            print_success(f"Found {setter} method")
        else:
            print_error(f"Missing {setter} method")
            checks_passed = False

    # Check for include of igrill_const.h
    if '#include "igrill_const.h"' in content:
        print_success("Includes igrill_const.h")
    else:
        print_error("Missing include of igrill_const.h")
        checks_passed = False

    return checks_passed

def test_init_py_content():
    """Test P1.3: Validate __init__.py content"""
    print_header("Testing __init__.py Content")

    if not os.path.exists(INIT_PY):
        print_error("Cannot test content - file doesn't exist")
        return False

    with open(INIT_PY, 'r') as f:
        content = f.read()

    checks_passed = True

    # Check for required imports
    required_imports = [
        'esphome.codegen',
        'esphome.config_validation',
        'ble_client',
        'sensor',
    ]

    for imp in required_imports:
        if imp in content:
            print_success(f"Found import for {imp}")
        else:
            print_error(f"Missing import for {imp}")
            checks_passed = False

    # Check for namespace and class definition
    if 'igrill_client_ns' in content:
        print_success("Found namespace definition")
    else:
        print_error("Missing namespace definition")
        checks_passed = False

    if 'IGrillClient' in content:
        print_success("Found IGrillClient class reference")
    else:
        print_error("Missing IGrillClient class reference")
        checks_passed = False

    # Check for probe configurations
    probe_configs = ['CONF_PROBE1', 'CONF_PROBE2', 'CONF_PROBE3', 'CONF_PROBE4']
    for probe in probe_configs:
        if probe in content:
            print_success(f"Found {probe} configuration")
        else:
            print_error(f"Missing {probe} configuration")
            checks_passed = False

    # Check for CONFIG_SCHEMA
    if 'CONFIG_SCHEMA' in content:
        print_success("Found CONFIG_SCHEMA")
    else:
        print_error("Missing CONFIG_SCHEMA")
        checks_passed = False

    # Check for to_code function
    if 'async def to_code' in content or 'def to_code' in content:
        print_success("Found to_code function")
    else:
        print_error("Missing to_code function")
        checks_passed = False

    return checks_passed

def test_python_syntax():
    """Test that __init__.py has valid Python syntax"""
    print_header("Testing Python Syntax")

    if not os.path.exists(INIT_PY):
        print_error("Cannot test syntax - file doesn't exist")
        return False

    try:
        with open(INIT_PY, 'r') as f:
            code = f.read()
        compile(code, INIT_PY, 'exec')
        print_success("__init__.py has valid Python syntax")
        return True
    except SyntaxError as e:
        print_error(f"Syntax error in __init__.py: {e}")
        return False

def main():
    print(f"\n{BOLD}{'='*60}{RESET}")
    print(f"{BOLD}Phase 1 - Core Component Structure Tests{RESET}")
    print(f"{BOLD}{'='*60}{RESET}")

    print_info(f"Base directory: {BASE_DIR}")
    print_info(f"Component directory: {COMPONENT_DIR}")

    results = {}

    # Run all tests
    results['existence'] = test_file_existence()
    results['const_h'] = test_const_h_content()
    results['client_h'] = test_client_h_content()
    results['init_py'] = test_init_py_content()
    results['syntax'] = test_python_syntax()

    # Summary
    print_header("Test Summary")

    total_tests = len(results)
    passed_tests = sum(1 for v in results.values() if v)

    print(f"\nTests Passed: {passed_tests}/{total_tests}")

    for test_name, passed in results.items():
        status = f"{GREEN}PASSED{RESET}" if passed else f"{RED}FAILED{RESET}"
        print(f"  {test_name}: {status}")

    if all(results.values()):
        print(f"\n{GREEN}{BOLD}✓ All Phase 1 tests PASSED!{RESET}")
        return 0
    else:
        print(f"\n{RED}{BOLD}✗ Some tests FAILED{RESET}")
        return 1

if __name__ == "__main__":
    sys.exit(main())
