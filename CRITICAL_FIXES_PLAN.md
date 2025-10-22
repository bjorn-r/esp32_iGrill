# Plan: Fix Critical Issues in ESPHome iGrill Implementation

## Overview
Address the 6 critical issues preventing the iGrill component from functioning correctly, plus sanitize configuration files.

## Priority 1: Fix Authentication State Machine (CRITICAL)

### Issue
Authentication returns success immediately without waiting for async BLE responses, causing premature characteristic discovery.

### Changes Required

**File: `components/igrill_client/igrill_client.h`**
- ✅ `AuthState` enum already defined - will use it
- Add public method: `void handle_auth_event(esp_gattc_cb_event_t event, esp_ble_gattc_cb_param_t *param)`

**File: `components/igrill_client/igrill_client.cpp`**
- Modify `authenticate_()` method:
  - Change from synchronous to async initialization
  - Set `auth_state_ = AUTH_CHALLENGE_SENT` instead of `authenticated_ = true`
  - Don't call `discover_characteristics_()` immediately

- Enhance `gattc_event_handler()` to handle authentication events:
  - Add `case ESP_GATTC_WRITE_CHAR_EVT:` - handles APP_CHALLENGE write completion
  - Add `case ESP_GATTC_READ_CHAR_EVT:` - handles DEVICE_CHALLENGE read completion
  - Add state machine transitions:
    - `AUTH_CHALLENGE_SENT` → trigger `read_device_challenge_()`
    - `AUTH_WAITING_DEVICE_CHALLENGE` → copy data, trigger `send_device_response_()`
    - `AUTH_RESPONSE_SENT` → verify completion
    - `AUTH_AUTHENTICATED` → call `discover_characteristics_()`

**File: `components/igrill_client/igrill_auth.h`**
- Add public methods to support async flow:
  - `void handle_write_complete(uint16_t handle, esp_gatt_status_t status)`
  - `void handle_read_complete(const uint8_t *data, uint16_t length)`
- Store characteristic handles for event correlation

**File: `components/igrill_client/igrill_auth.cpp`**
- Modify `authenticate()`: only initiate APP_CHALLENGE write, don't return final status
- Modify `read_device_challenge_()`: store request handle for event matching
- Modify `send_device_response_()`: don't set `authenticated_` here
- Add new methods to handle async callbacks

## Priority 2: Fix ESP-IDF Status Check (CRITICAL)

### Issue
ESP-IDF returns 0 (ESP_GATT_OK) for success, but code checks `if (status)` which is inverted.

### Changes Required

**File: `components/igrill_client/igrill_client.cpp`**
- Line ~217: Change from:
  ```cpp
  if (status) {
    ESP_LOGD(TAG, "Successfully subscribed...");
  }
  ```
  To:
  ```cpp
  if (status == ESP_GATT_OK) {
    ESP_LOGD(TAG, "Successfully subscribed...");
  } else {
    ESP_LOGW(TAG, "Failed to subscribe to characteristic handle 0x%04x (error: %d)", pair.handle, status);
  }
  ```

## Priority 3: Add CCCD Writes for Notifications (CRITICAL)

### Issue
Calling `esp_ble_gattc_register_for_notify()` alone doesn't enable notifications - must also write 0x0001 to CCCD descriptor.

### Changes Required

**File: `components/igrill_client/igrill_client.cpp`**
- Modify `subscribe_to_characteristics_()`:
  - After `esp_ble_gattc_register_for_notify()` succeeds
  - Write 0x0001 to CCCD (handle = characteristic_handle + 1, or discover it)
  - Use `esp_ble_gattc_write_char_descr()` with descriptor handle

**Implementation approach:**
```cpp
// After successful register_for_notify:
uint16_t notify_enable = 0x0001;
uint16_t cccd_handle = pair.handle + 1; // or discover via get_descriptor()
esp_ble_gattc_write_char_descr(
  client->get_gattc_if(),
  client->get_conn_id(),
  cccd_handle,
  sizeof(notify_enable),
  (uint8_t*)&notify_enable,
  ESP_GATT_WRITE_TYPE_RSP,
  ESP_GATT_AUTH_REQ_NONE
);
```

## Priority 4: Fix Battery Service UUID (CRITICAL)

### Issue
Using `"0000"` as service UUID is invalid. Battery characteristic (0x2A19) belongs to Battery Service (0x180F).

### Changes Required

**File: `components/igrill_client/igrill_const.h`**
- Add constant:
  ```cpp
  static const char *const BATTERY_SERVICE_UUID = "0000180f-0000-1000-8000-00805f9b34fb";
  ```

**File: `components/igrill_client/igrill_client.cpp`**
- Line ~158-161: Change from:
  ```cpp
  auto battery_char = client->get_characteristic(
    esphome::esp32_ble::ESPBTUUID::from_raw("0000"),
    esphome::esp32_ble::ESPBTUUID::from_raw(BATTERY_UUID)
  );
  ```
  To:
  ```cpp
  auto battery_char = client->get_characteristic(
    esphome::esp32_ble::ESPBTUUID::from_raw(BATTERY_SERVICE_UUID),
    esphome::esp32_ble::ESPBTUUID::from_raw(BATTERY_UUID)
  );
  ```

## Priority 5: Fix Temperature Parsing (CRITICAL)

### Issue
Temperature values are read as raw int16 without division, likely causing incorrect readings (off by 10x or 100x).

### Changes Required

**File: `components/igrill_client/igrill_client.cpp`**
- Line ~235-240: Research iGrill protocol to determine correct scaling
- Add division factor (likely /10.0 or /100.0):
  ```cpp
  int16_t raw_temp = (data[1] << 8) | data[0];
  float temp_celsius = raw_temp / 10.0f; // Verify with real hardware!
  ```

**Testing requirement:** This MUST be verified with actual iGrill device and known temperature values.

## Priority 6: Sanitize Secrets File (SECURITY)

### Issue
`examples/secrets.yaml` contains realistic-looking credentials that could be real API keys.

### Changes Required

**File: `examples/secrets.yaml`**
- Replace all values with obvious placeholders:
  ```yaml
  wifi_ssid: "YOUR_WIFI_SSID"
  wifi_password: "YOUR_WIFI_PASSWORD"
  igrill_mac_address: "AA:BB:CC:DD:EE:FF"
  api_encryption_key: "GENERATE_WITH_COMMAND_BELOW"
  ota_password: "CHOOSE_SECURE_PASSWORD"
  ```
- Add comment: `# Generate API key with: esphome config examples/igrill-basic.yaml`

**File: `.gitignore`**
- Verify `secrets.yaml` is listed (already present in examples/.gitignore)

## Priority 7: Clean Up YAML Configuration (MEDIUM)

### Issue
YAML files reference non-existent configuration parameters that aren't implemented in the component.

### Changes Required

**File: `examples/igrill-full.yaml`**
- Remove unimplemented parameters:
  - Line ~72: Remove `scan_interval: 60s` (not in component schema)
  - Line ~73: Remove `timeout: 30s` (not in component schema)

**File: `examples/igrill-v3-propane.yaml`**
- Remove unimplemented parameters:
  - Line ~46: Remove `device_type: igrill_v3` (not validated/used)
  - Lines ~62-63: Remove `low_threshold` and `high_threshold` (not implemented)

## Additional Improvements (OPTIONAL)

### Add Named Constants
**File: `components/igrill_client/igrill_const.h`**
- Add: `static const uint8_t PROBE_DISCONNECTED_FLAG = 0xF8;`
- Add: `static const uint8_t PROPANE_PERCENTAGE_MULTIPLIER = 25;`

### Add Authentication Timeout
**File: `components/igrill_client/igrill_client.h`**
- Add member: `uint32_t auth_timeout_ms_{10000};`
- Add member: `uint32_t auth_start_time_{0};`

**File: `components/igrill_client/igrill_client.cpp`**
- In `update()`: Check if authentication timed out and reset state

### Remove Duplicate Struct Definitions
**File: `components/igrill_client/igrill_client.cpp`**
- Lines ~67-70 and ~195-198: Remove local `HandleSensorPair` definitions
- Use the header definition exclusively

## Implementation Order

1. **Fix secrets.yaml** (5 min) - Security issue, quick fix
2. **Fix Battery Service UUID** (5 min) - Simple constant change
3. **Fix ESP-IDF status checks** (10 min) - Simple logic fix
4. **Fix temperature parsing** (15 min) - Add scaling factor, document need for testing
5. **Clean up YAML files** (10 min) - Remove non-existent params
6. **Implement authentication state machine** (2-3 hours) - Complex async refactoring
7. **Add CCCD writes** (1 hour) - Research descriptor handling, implement writes

**Total estimated time: 4-5 hours**

## Testing Requirements

After implementation:
1. Build and flash to ESP32 hardware
2. Verify authentication completes successfully (check logs for state transitions)
3. Verify notifications are received (check logs for characteristic updates)
4. **Verify temperature readings with known values** (critical - scaling must be correct)
5. Verify battery level reporting
6. Test probe connect/disconnect detection

## Risks & Unknowns

- **Temperature scaling factor**: Unknown without protocol documentation or hardware testing
- **CCCD handle discovery**: May need descriptor enumeration if handle != char_handle + 1
- **Authentication protocol details**: May have additional steps not in current implementation
- **iGrill device variations**: Different models may have protocol differences

## Success Criteria

- ✅ Authentication state machine completes all 3 steps asynchronously
- ✅ Notifications are received for all configured characteristics
- ✅ Temperature readings are accurate (within ±1°C of actual)
- ✅ Battery level displays correctly
- ✅ No crashes or memory leaks during operation
- ✅ Configuration validates without warnings
- ✅ Secrets file contains only placeholder values

## Code Review Summary Reference

Based on comprehensive code review findings:
- **Critical Issues**: 6 identified (authentication, notifications, UUID, parsing)
- **Major Issues**: 4 identified (race conditions, config params, hardcoded values)
- **Minor Issues**: 10+ identified (logging, constants, null checks)
- **Overall Assessment**: "DO NOT deploy in current state" - core functionality broken
- **Positive Aspects**: Good architecture, clean structure, proper ESPHome integration

## Notes for Implementation

- Test each fix incrementally with hardware before moving to next
- Add comprehensive logging at each authentication state transition
- Consider adding debug flags for verbose BLE event logging
- Document any iGrill protocol assumptions or reverse-engineered behaviors
- Keep authentication timeout configurable for different iGrill models
