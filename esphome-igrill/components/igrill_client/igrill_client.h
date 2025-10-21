#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"
#include "igrill_const.h"

#ifdef USE_ESP32

namespace esphome {
namespace igrill_client {

class IGrillClient : public esphome::ble_client::BLEClientNode, public PollingComponent {
 public:
  IGrillClient() = default;

  void setup() override;
  void update() override;
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                          esp_ble_gattc_cb_param_t *param) override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Probe temperature sensors
  void set_probe1_sensor(sensor::Sensor *sensor) { this->probe1_sensor_ = sensor; }
  void set_probe2_sensor(sensor::Sensor *sensor) { this->probe2_sensor_ = sensor; }
  void set_probe3_sensor(sensor::Sensor *sensor) { this->probe3_sensor_ = sensor; }
  void set_probe4_sensor(sensor::Sensor *sensor) { this->probe4_sensor_ = sensor; }

  // Battery sensor
  void set_battery_sensor(sensor::Sensor *sensor) { this->battery_sensor_ = sensor; }

  // Propane sensor (for iGrill mini)
  void set_propane_sensor(sensor::Sensor *sensor) { this->propane_sensor_ = sensor; }

 protected:
  // Sensor pointers
  sensor::Sensor *probe1_sensor_{nullptr};
  sensor::Sensor *probe2_sensor_{nullptr};
  sensor::Sensor *probe3_sensor_{nullptr};
  sensor::Sensor *probe4_sensor_{nullptr};
  sensor::Sensor *battery_sensor_{nullptr};
  sensor::Sensor *propane_sensor_{nullptr};

  // BLE characteristic handles
  uint16_t probe1_handle_{0};
  uint16_t probe2_handle_{0};
  uint16_t probe3_handle_{0};
  uint16_t probe4_handle_{0};
  uint16_t battery_handle_{0};
  uint16_t propane_handle_{0};

  // Authentication handles
  uint16_t app_challenge_handle_{0};
  uint16_t device_challenge_handle_{0};
  uint16_t device_response_handle_{0};

  // State tracking
  bool authenticated_{false};
  bool subscribed_{false};

  // Helper methods
  void authenticate_();
  void subscribe_to_characteristics_();
  void read_probe_temperature_(uint16_t handle, sensor::Sensor *sensor);
  void read_battery_level_();
  void read_propane_level_();
  float parse_temperature_(const uint8_t *data, uint16_t length);
  uint8_t parse_battery_(const uint8_t *data, uint16_t length);
};

}  // namespace igrill_client
}  // namespace esphome

#endif  // USE_ESP32
