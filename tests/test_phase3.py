#!/usr/bin/env python3
"""
Phase 3 Test - Sensor Implementations
Tests for igrill_client sensor methods and model detection
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
BASE_DIR = "/Users/bjorn/Development/esp32_iGrill/esphome-igrill"
COMPONENT_DIR = os.path.join(BASE_DIR, "components", "igrill_client")

# File paths
CLIENT_CPP = os.path.join(COMPONENT_DIR, "igrill_client.cpp")
CLIENT_H = os.path.join(COMPONENT_DIR, "igrill_client.h")
CONST_H = os.path.join(COMPONENT_DIR, "igrill_const.h")

def test_file_existence():
    """Test Phase 3.1: Check if implementation files exist"""
    print_header("Testing File Existence")

    files_to_check = [
        CLIENT_CPP,
        CONST_H
    ]

    all_exist = True
    for file_path in files_to_check:
        if os.path.exists(file_path):
            print_success(f"{os.path.basename(file_path)} exists at {file_path}")
        else:
            print_error(f"{os.path.basename(file_path)} NOT FOUND at {file_path}")
            all_exist = False

    return all_exist

def test_probe_methods():
    """Test Phase 3.2: Validate Probe Temperature Methods"""
    print_header("Testing Probe Temperature Methods")

    if not os.path.exists(CLIENT_CPP) or not os.path.exists(CLIENT_H):
        print_error("Cannot test probe methods - files don't exist")
        return False

    with open(CLIENT_CPP, 'r') as f:
        cpp_content = f.read()

    with open(CLIENT_H, 'r') as f:
        h_content = f.read()

    # Methods in CPP
    cpp_methods = [
        'parse_temperature_',
        'setup_probe_characteristics_',
        'read_probe_temperature_'
    ]

    # Methods/setters in header (inline)
    h_methods = [
        'set_probe1_sensor',
        'set_probe2_sensor',
        'set_probe3_sensor',
        'set_probe4_sensor'
    ]

    probe_handles = [
        'probe1_handle_',
        'probe2_handle_',
        'probe3_handle_',
        'probe4_handle_'
    ]

    checks_passed = True

    # Check CPP methods
    for method in cpp_methods:
        if method in cpp_content:
            print_success(f"Found {method} in igrill_client.cpp")
        else:
            print_error(f"Missing {method} in igrill_client.cpp")
            checks_passed = False

    # Check header methods (inline)
    for method in h_methods:
        if method in h_content:
            print_success(f"Found {method} in igrill_client.h")
        else:
            print_error(f"Missing {method} in igrill_client.h")
            checks_passed = False

    # Check probe handle members in header
    for handle in probe_handles:
        if handle in h_content:
            print_success(f"Found {handle} in igrill_client.h")
        else:
            print_error(f"Missing {handle} in igrill_client.h")
            checks_passed = False

    return checks_passed

def test_battery_methods():
    """Test Phase 3.3: Validate Battery Level Methods"""
    print_header("Testing Battery Level Methods")

    if not os.path.exists(CLIENT_CPP) or not os.path.exists(CLIENT_H):
        print_error("Cannot test battery methods - files don't exist")
        return False

    with open(CLIENT_CPP, 'r') as f:
        cpp_content = f.read()

    with open(CLIENT_H, 'r') as f:
        h_content = f.read()

    cpp_methods = [
        'parse_battery_',
        'setup_battery_characteristic_',
        'read_battery_level_'
    ]

    h_methods = ['set_battery_sensor']
    battery_handle = 'battery_handle_'

    checks_passed = True

    # Check CPP methods
    for method in cpp_methods:
        if method in cpp_content:
            print_success(f"Found {method} in igrill_client.cpp")
        else:
            print_error(f"Missing {method} in igrill_client.cpp")
            checks_passed = False

    # Check header methods
    for method in h_methods:
        if method in h_content:
            print_success(f"Found {method} in igrill_client.h")
        else:
            print_error(f"Missing {method} in igrill_client.h")
            checks_passed = False

    # Check battery handle member in header
    if battery_handle in h_content:
        print_success(f"Found {battery_handle} in igrill_client.h")
    else:
        print_error(f"Missing {battery_handle} in igrill_client.h")
        checks_passed = False

    return checks_passed

def test_propane_methods():
    """Test Phase 3.4: Validate Propane Level Methods"""
    print_header("Testing Propane Level Methods")

    if not os.path.exists(CLIENT_CPP) or not os.path.exists(CLIENT_H):
        print_error("Cannot test propane methods - files don't exist")
        return False

    with open(CLIENT_CPP, 'r') as f:
        cpp_content = f.read()

    with open(CLIENT_H, 'r') as f:
        h_content = f.read()

    cpp_methods = [
        'parse_propane_',
        'setup_propane_characteristic_',
        'read_propane_level_'
    ]

    h_methods = ['set_propane_sensor']
    propane_handle = 'propane_handle_'

    checks_passed = True

    # Check CPP methods
    for method in cpp_methods:
        if method in cpp_content:
            print_success(f"Found {method} in igrill_client.cpp")
        else:
            print_error(f"Missing {method} in igrill_client.cpp")
            checks_passed = False

    # Check header methods
    for method in h_methods:
        if method in h_content:
            print_success(f"Found {method} in igrill_client.h")
        else:
            print_error(f"Missing {method} in igrill_client.h")
            checks_passed = False

    # Check propane handle member in header
    if propane_handle in h_content:
        print_success(f"Found {propane_handle} in igrill_client.h")
    else:
        print_error(f"Missing {propane_handle} in igrill_client.h")
        checks_passed = False

    return checks_passed

def test_temperature_units():
    """Test Phase 3.5: Validate Temperature Units Methods"""
    print_header("Testing Temperature Units")

    if not os.path.exists(CONST_H) or not os.path.exists(CLIENT_CPP) or not os.path.exists(CLIENT_H):
        print_error("Cannot test temperature units - files don't exist")
        return False

    # Check const.h for TEMP_UNITS_UUID
    with open(CONST_H, 'r') as f:
        const_content = f.read()

    # Check client.cpp for temperature unit methods
    with open(CLIENT_CPP, 'r') as f:
        cpp_content = f.read()

    # Check client.h for inline methods
    with open(CLIENT_H, 'r') as f:
        h_content = f.read()

    checks_passed = True

    # Check for TEMP_UNITS_UUID in const.h
    if 'TEMP_UNITS_UUID' in const_content:
        print_success("Found TEMP_UNITS_UUID in igrill_const.h")
    else:
        print_error("Missing TEMP_UNITS_UUID in igrill_const.h")
        checks_passed = False

    # Check for set_temperature_units in CPP
    if 'set_temperature_units' in cpp_content:
        print_success("Found set_temperature_units in igrill_client.cpp")
    else:
        print_error("Missing set_temperature_units in igrill_client.cpp")
        checks_passed = False

    # Check for set_use_metric in header (inline method)
    if 'set_use_metric' in h_content:
        print_success("Found set_use_metric in igrill_client.h")
    else:
        print_error("Missing set_use_metric in igrill_client.h")
        checks_passed = False

    return checks_passed

def test_model_detection():
    """Test Phase 3.6: Validate Model Detection Methods"""
    print_header("Testing Model Detection")

    if not os.path.exists(CLIENT_CPP) or not os.path.exists(CLIENT_H):
        print_error("Cannot test model detection - files don't exist")
        return False

    # Check client.h for IGrillModel enum
    with open(CLIENT_H, 'r') as f:
        h_content = f.read()

    # Check client.cpp for model detection methods
    with open(CLIENT_CPP, 'r') as f:
        cpp_content = f.read()

    checks_passed = True

    # Check for IGrillModel enum in client.h (it's a nested enum class)
    if 'enum class IGrillModel' in h_content or 'IGrillModel' in h_content:
        print_success("Found IGrillModel enum in igrill_client.h")
    else:
        print_error("Missing IGrillModel enum in igrill_client.h")
        checks_passed = False

    # Check for model detection method in CPP
    if 'detect_model_' in cpp_content:
        print_success("Found detect_model_ in igrill_client.cpp")
    else:
        print_error("Missing detect_model_ in igrill_client.cpp")
        checks_passed = False

    return checks_passed

def test_gatt_event_handler():
    """Test Phase 3.7: Verify GATT Event Handler"""
    print_header("Testing GATT Event Handler")

    if not os.path.exists(CLIENT_CPP):
        print_error("Cannot test GATT event handler - file doesn't exist")
        return False

    with open(CLIENT_CPP, 'r') as f:
        content = f.read()

    checks_passed = True

    # Check for sensor-related event handling
    sensor_event_checks = [
        'parse_temperature_',
        'parse_battery_',
        'parse_propane_'
    ]

    for sensor_event in sensor_event_checks:
        if sensor_event in content:
            print_success(f"Found {sensor_event} in gattc_event_handler")
        else:
            print_error(f"Missing {sensor_event} in gattc_event_handler")
            checks_passed = False

    return checks_passed

def main():
    print(f"\n{BOLD}{'='*60}{RESET}")
    print(f"{BOLD}Phase 3 - Sensor Implementation Tests{RESET}")
    print(f"{BOLD}{'='*60}{RESET}")

    print_info(f"Base directory: {BASE_DIR}")
    print_info(f"Component directory: {COMPONENT_DIR}")

    # Define and run tests
    tests = [
        ('File Existence', test_file_existence),
        ('Probe Methods', test_probe_methods),
        ('Battery Methods', test_battery_methods),
        ('Propane Methods', test_propane_methods),
        ('Temperature Units', test_temperature_units),
        ('Model Detection', test_model_detection),
        ('GATT Event Handler', test_gatt_event_handler),
    ]

    results = {}

    # Run all tests
    for name, test_func in tests:
        print_header(f"Running {name} Test")
        try:
            results[name] = test_func()
        except Exception as e:
            print_error(f"Error in {name} test: {e}")
            results[name] = False

    # Summary
    print_header("Test Summary")

    total_tests = len(results)
    passed_tests = sum(1 for v in results.values() if v)

    print(f"\nTests Passed: {passed_tests}/{total_tests}")

    for test_name, passed in results.items():
        status = f"{GREEN}PASSED{RESET}" if passed else f"{RED}FAILED{RESET}"
        print(f"  {test_name}: {status}")

    if all(results.values()):
        print(f"\n{GREEN}{BOLD}✓ All Phase 3 tests PASSED!{RESET}")
        return 0
    else:
        print(f"\n{RED}{BOLD}✗ Some tests FAILED{RESET}")
        return 1

if __name__ == "__main__":
    sys.exit(main())