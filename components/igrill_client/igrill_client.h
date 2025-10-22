#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/esp32_ble/ble_uuid.h"
#include "igrill_const.h"
#include "igrill_auth.h"

#ifdef USE_ESP32

namespace esphome {
namespace igrill_client {

// Authentication state machine
enum AuthState {
  AUTH_IDLE,
  AUTH_CHALLENGE_SENT,
  AUTH_WAITING_DEVICE_CHALLENGE,
  AUTH_RESPONSE_SENT,
  AUTH_AUTHENTICATED,
  AUTH_FAILED
};

class IGrillClient : public ble_client::BLEClientNode, public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                          esp_ble_gattc_cb_param_t *param) override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Sensor setters
  void set_probe1_sensor(sensor::Sensor *sensor) { this->probe1_sensor_ = sensor; }
  void set_probe2_sensor(sensor::Sensor *sensor) { this->probe2_sensor_ = sensor; }
  void set_probe3_sensor(sensor::Sensor *sensor) { this->probe3_sensor_ = sensor; }
  void set_probe4_sensor(sensor::Sensor *sensor) { this->probe4_sensor_ = sensor; }
  void set_battery_sensor(sensor::Sensor *sensor) { this->battery_sensor_ = sensor; }
  void set_propane_sensor(sensor::Sensor *sensor) { this->propane_sensor_ = sensor; }

  // Configuration
  void set_use_metric(bool use_metric) { this->use_metric_ = use_metric; }

 protected:
  // Helpers for sensor-handle mapping
  struct HandleSensorPair {
    uint16_t handle;
    sensor::Sensor* sensor;
  };

  // Sensor pointers
  sensor::Sensor *probe1_sensor_{nullptr};
  sensor::Sensor *probe2_sensor_{nullptr};
  sensor::Sensor *probe3_sensor_{nullptr};
  sensor::Sensor *probe4_sensor_{nullptr};
  sensor::Sensor *battery_sensor_{nullptr};
  sensor::Sensor *propane_sensor_{nullptr};

  // Configuration
  bool use_metric_{false};
  bool authenticated_{false};

  // BLE characteristic handles
  uint16_t probe1_handle_{0};
  uint16_t probe2_handle_{0};
  uint16_t probe3_handle_{0};
  uint16_t probe4_handle_{0};
  uint16_t battery_handle_{0};
  uint16_t propane_handle_{0};

  // Authentication
  IgrillAuthenticator authenticator_;
  AuthState auth_state_{AUTH_IDLE};

  // Authentication timeout tracking
  uint32_t auth_start_time_{0};
  uint32_t auth_timeout_ms_{10000};  // 10 second timeout

  // Helper methods
  void authenticate_();
  void subscribe_to_characteristics_();
  void discover_characteristics_();
  float parse_temperature_(const uint8_t *data, uint16_t length);
  uint8_t parse_battery_(const uint8_t *data, uint16_t length);
  uint8_t parse_propane_(const uint8_t *data, uint16_t length);
};

}  // namespace igrill_client
}  // namespace esphome

#endif  // USE_ESP32
