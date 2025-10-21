#!/usr/bin/env python3
"""
Phase 4 Test - YAML Configuration and Initialization
Tests for iGrill ESPHome YAML files and __init__.py updates
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
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
COMPONENT_DIR = os.path.join(BASE_DIR, "components", "igrill_client")
EXAMPLES_DIR = os.path.join(BASE_DIR, "examples")
INIT_PY = os.path.join(COMPONENT_DIR, "__init__.py")

def test_example_yaml_files():
    """Test P4.1: Check existence of YAML example files"""
    print_header("Testing Example YAML Files")

    example_files = [
        "secrets.yaml.example",
        "igrill-basic.yaml",
        "igrill-full.yaml",
        "igrill-v3-propane.yaml"
    ]

    all_exist = True
    for filename in example_files:
        filepath = os.path.join(EXAMPLES_DIR, filename)
        if os.path.exists(filepath):
            print_success(f"{filename} exists")
        else:
            print_error(f"{filename} NOT FOUND")
            all_exist = False

    return all_exist

def test_secrets_yaml_example():
    """Test P4.2: Validate secrets.yaml.example content"""
    print_header("Testing secrets.yaml.example Content")

    secrets_path = os.path.join(EXAMPLES_DIR, "secrets.yaml.example")
    if not os.path.exists(secrets_path):
        print_error("secrets.yaml.example not found")
        return False

    with open(secrets_path, 'r') as f:
        content = f.read()

    required_secrets = [
        "wifi_ssid",
        "wifi_password",
        "igrill_mac_address"
    ]

    optional_secrets = [
        "igrill_encryption_key"
    ]

    checks_passed = True
    for secret in required_secrets:
        if secret in content:
            print_success(f"Found {secret}")
        else:
            print_error(f"Missing {secret}")
            checks_passed = False

    for secret in optional_secrets:
        if secret in content:
            print_success(f"Optional {secret} found")

    return checks_passed

def test_yaml_configuration():
    """Test P4.3: Validate YAML configuration sections"""
    print_header("Testing YAML Configuration Sections")

    yaml_files = [
        "igrill-basic.yaml",
        "igrill-full.yaml",
        "igrill-v3-propane.yaml"
    ]

    checks_passed = True
    for filename in yaml_files:
        filepath = os.path.join(EXAMPLES_DIR, filename)

        if not os.path.exists(filepath):
            print_error(f"Cannot test {filename} - file does not exist")
            checks_passed = False
            continue

        with open(filepath, 'r') as f:
            content = f.read()

        required_sections = [
            "esphome:",
            "esp32:",
            "esp32_ble_tracker:",
            "ble_client:",
            "igrill_client:"
        ]

        for section in required_sections:
            if section in content:
                print_success(f"{filename} contains {section.strip()} section")
            else:
                print_error(f"{filename} missing {section.strip()} section")
                checks_passed = False

        # Check for mac_address and use_metric
        if "mac_address:" in content and "use_metric:" in content:
            print_success(f"{filename} contains mac_address and use_metric")
        else:
            print_error(f"{filename} missing mac_address or use_metric")
            checks_passed = False

    return checks_passed

def test_init_py_updates():
    """Test P4.4: Validate __init__.py updates"""
    print_header("Testing __init__.py Updates")

    if not os.path.exists(INIT_PY):
        print_error("Cannot test __init__.py - file does not exist")
        return False

    with open(INIT_PY, 'r') as f:
        content = f.read()

    checks_passed = True

    # Check new imports and dependencies
    required_imports = [
        "CONF_USE_METRIC",
        "AUTO_LOAD"
    ]

    for imp in required_imports:
        if imp in content:
            print_success(f"Found {imp}")
        else:
            print_error(f"Missing {imp}")
            checks_passed = False

    # Check configuration schema updates
    config_checks = [
        "use_metric",
        "set_use_metric"
    ]

    for check in config_checks:
        if check in content:
            print_success(f"Found {check}")
        else:
            print_error(f"Missing {check}")
            checks_passed = False

    return checks_passed

def main():
    print(f"\n{BOLD}{'='*60}{RESET}")
    print(f"{BOLD}Phase 4 - YAML Configuration and Initialization Tests{RESET}")
    print(f"{BOLD}{'='*60}{RESET}")

    print_info(f"Base directory: {BASE_DIR}")
    print_info(f"Component directory: {COMPONENT_DIR}")
    print_info(f"Examples directory: {EXAMPLES_DIR}")

    results = {}

    # Run all tests
    results['example_yaml_files'] = test_example_yaml_files()
    results['secrets_yaml'] = test_secrets_yaml_example()
    results['yaml_configuration'] = test_yaml_configuration()
    results['init_py_updates'] = test_init_py_updates()

    # Summary
    print_header("Test Summary")

    total_tests = len(results)
    passed_tests = sum(1 for v in results.values() if v)

    print(f"\nTests Passed: {passed_tests}/{total_tests}")

    for test_name, passed in results.items():
        status = f"{GREEN}PASSED{RESET}" if passed else f"{RED}FAILED{RESET}"
        print(f"  {test_name}: {status}")

    if all(results.values()):
        print(f"\n{GREEN}{BOLD}✓ All Phase 4 tests PASSED!{RESET}")
        return 0
    else:
        print(f"\n{RED}{BOLD}✗ Some tests FAILED{RESET}")
        return 1

if __name__ == "__main__":
    sys.exit(main())