#!/usr/bin/env python3
"""Test Phase 2: BLE Authentication module"""

import os
import sys

def test_auth_files_exist():
    """Test that authentication files exist"""
    base = "/home/user/esp32_iGrill/esphome-igrill/components/igrill_client"

    files = ["igrill_auth.h", "igrill_auth.cpp"]

    for f in files:
        path = os.path.join(base, f)
        if not os.path.exists(path):
            print(f"✗ Missing: {f}")
            return False
        print(f"✓ Found: {f}")

    return True

def test_auth_header_content():
    """Test igrill_auth.h has required elements"""
    path = "/home/user/esp32_iGrill/esphome-igrill/components/igrill_client/igrill_auth.h"

    with open(path, 'r') as f:
        content = f.read()

    required = [
        "class IgrillAuthenticator",
        "static bool authenticate",
        "app_challenge_handle",
        "device_challenge_handle",
        "device_response_handle"
    ]

    for item in required:
        if item not in content:
            print(f"✗ Missing in header: {item}")
            return False
        print(f"✓ Found in header: {item}")

    return True

def test_auth_cpp_content():
    """Test igrill_auth.cpp has required implementation"""
    path = "/home/user/esp32_iGrill/esphome-igrill/components/igrill_client/igrill_auth.cpp"

    with open(path, 'r') as f:
        content = f.read()

    required = [
        "bool IgrillAuthenticator::authenticate",
        "write_app_challenge_",
        "read_device_challenge_",
        "write_device_response_",
        "AUTH_CHALLENGE",
        "ESP_LOGI",
        "ESP_LOGE"
    ]

    for item in required:
        if item not in content:
            print(f"✗ Missing in cpp: {item}")
            return False
        print(f"✓ Found in cpp: {item}")

    return True

if __name__ == "__main__":
    print("="*60)
    print("Phase 2: BLE Authentication Tests")
    print("="*60)

    tests = [
        ("File Existence", test_auth_files_exist),
        ("Header Content", test_auth_header_content),
        ("CPP Content", test_auth_cpp_content),
    ]

    passed = 0
    failed = 0

    for name, test_func in tests:
        print(f"\n{name}:")
        print("-"*60)
        try:
            if test_func():
                passed += 1
                print(f"✓ {name}: PASSED")
            else:
                failed += 1
                print(f"✗ {name}: FAILED")
        except Exception as e:
            failed += 1
            print(f"✗ {name}: ERROR - {e}")

    print("\n" + "="*60)
    print(f"Results: {passed} passed, {failed} failed")
    print("="*60)

    sys.exit(0 if failed == 0 else 1)
