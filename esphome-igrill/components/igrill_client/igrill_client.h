#pragma once

#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/sensor/sensor.h"
#include "igrill_const.h"

namespace esphome {
namespace igrill_client {

class IGrillClient : public ble_client::BLEClient {
 public:
  void set_temperature_units();
  void set_use_metric(bool use_metric) { use_metric_ = use_metric; }
  void set_battery_sensor(sensor::Sensor *battery_sensor) { battery_sensor_ = battery_sensor; }
  void set_propane_sensor(sensor::Sensor *propane_sensor) { propane_sensor_ = propane_sensor; }

 private:
  bool use_metric_ = false;

  // iGrill Model Detection
  enum class IGrillModel {
    UNKNOWN,
    MINI,
    MINI_V2,
    V2,
    V202,
    V3
  };
  IGrillModel igrill_model_ = IGrillModel::UNKNOWN;
  std::string igrill_model_name_ = "Unknown";

  // Battery-related members
  sensor::Sensor *battery_sensor_ = nullptr;
  uint16_t battery_handle_ = 0;

  // Propane-related members
  sensor::Sensor *propane_sensor_ = nullptr;
  uint16_t propane_handle_ = 0;

  // Battery handling methods
  uint8_t parse_battery_(const uint8_t *data, uint16_t length);
  void read_battery_level_();
  void setup_battery_characteristic_();

  // Propane handling methods
  uint8_t parse_propane_(const uint8_t *data, uint16_t length);
  void read_propane_level_();
  void setup_propane_characteristic_();

  // Probe-related members
  void set_probe1_sensor(sensor::Sensor *probe1_sensor) { probe1_sensor_ = probe1_sensor; }
  void set_probe2_sensor(sensor::Sensor *probe2_sensor) { probe2_sensor_ = probe2_sensor; }
  void set_probe3_sensor(sensor::Sensor *probe3_sensor) { probe3_sensor_ = probe3_sensor; }
  void set_probe4_sensor(sensor::Sensor *probe4_sensor) { probe4_sensor_ = probe4_sensor; }

  // Probe sensors and handles
  sensor::Sensor *probe1_sensor_ = nullptr;
  sensor::Sensor *probe2_sensor_ = nullptr;
  sensor::Sensor *probe3_sensor_ = nullptr;
  sensor::Sensor *probe4_sensor_ = nullptr;

  uint16_t probe1_handle_ = 0;
  uint16_t probe2_handle_ = 0;
  uint16_t probe3_handle_ = 0;
  uint16_t probe4_handle_ = 0;

  // Probe handling methods
  void read_probe_temperature_(uint16_t handle, sensor::Sensor *sensor);
  void setup_probe_characteristics_();

  // Model detection method
  void detect_model_();

  // Modified GATT event handler to include probe notifications
  void gattc_event_handler(
      esp_gattc_cb_event_t event,
      esp_gattc_cb_param_t *param,
      esp_gatt_if_t gatt_if
  ) override;

  // Existing authentication method updated to include probe setup
  void authenticate_() override;
};

}  // namespace igrill_client
}  // namespace esphome
