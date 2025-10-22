#pragma once

#include "esphome/core/helpers.h"

namespace esphome {
namespace igrill_client {

// iGrill BLE Service and Characteristic UUIDs
static const char *const IGRILL_SERVICE_UUID = "0000fff0-0000-1000-8000-00805f9b34fb";

// Authentication Service and Characteristic UUIDs
static const char *const AUTH_SERVICE_UUID = "64ac0000-4a4b-4b58-9f37-94d3c52ffdf7";
static const char *const APP_CHALLENGE_UUID = "64ac0002-4a4b-4b58-9f37-94d3c52ffdf7";
static const char *const DEVICE_CHALLENGE_UUID = "64ac0003-4a4b-4b58-9f37-94d3c52ffdf7";
static const char *const DEVICE_RESPONSE_UUID = "64ac0004-4a4b-4b58-9f37-94d3c52ffdf7";

// Probe characteristic UUIDs (for temperature readings)
static const char *const PROBE1_UUID = "06ef0002-2e06-4b79-9e33-fce2c42805ec";
static const char *const PROBE2_UUID = "06ef0004-2e06-4b79-9e33-fce2c42805ec";
static const char *const PROBE3_UUID = "06ef0006-2e06-4b79-9e33-fce2c42805ec";
static const char *const PROBE4_UUID = "06ef0008-2e06-4b79-9e33-fce2c42805ec";

// Battery characteristic UUID
static const char *const BATTERY_UUID = "00002a19-0000-1000-8000-00805f9b34fb";
// Battery Service UUID (standard Bluetooth SIG UUID)
static const char *const BATTERY_SERVICE_UUID = "0000180f-0000-1000-8000-00805f9b34fb";

// Propane level characteristic UUID (for iGrill mini)
static const char *const PROPANE_UUID = "06ef0003-2e06-4b79-9e33-fce2c42805ec";

// Constants
static const uint8_t IGRILL_MAX_PROBES = 4;
static const float INVALID_TEMPERATURE = -999.0f;
static const uint16_t INVALID_TEMPERATURE_RAW = 0xFFFF;

// Temperature conversion constants
// iGrill devices report temperature in tenths of degrees Celsius
// The raw int16 value must be divided by 10 to get actual temperature
static const float TEMP_SCALE_FACTOR = 10.0f;  // iGrill reports in tenths of degrees
static const float TEMP_OFFSET = 0.0f;
static const float TEMP_SCALE = 1.0f;

// Probe status
enum ProbeStatus {
  PROBE_STATUS_UNKNOWN = 0,
  PROBE_STATUS_CONNECTED = 1,
  PROBE_STATUS_DISCONNECTED = 2,
  PROBE_STATUS_ERROR = 3
};

// Temperature Units
static const char *const IGRILL_TEMP_UNITS_UUID = "06ef000a-2e06-4b79-9e33-fce2c42805ec";
static const uint8_t IMPERIAL_UNITS = 0;
static const uint8_t METRIC_UNITS = 1;

// Authentication challenge (16 zeros)
static const uint8_t AUTH_CHALLENGE[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

}  // namespace igrill_client
}  // namespace esphome
