#include "igrill_client.h"
#include "esphome/core/log.h"
#include "igrill_auth.h"
#include "igrill_const.h"

namespace esphome {
namespace igrill_client {

static const char *const TAG = "igrill_client";

void IGrillClient::setup() {
  ESP_LOGCONFIG(TAG, "Setting up iGrill Client...");
}

void IGrillClient::update() {
  // Polling update - check connection status
  if (this->node_state == esp32_ble_tracker::ClientState::ESTABLISHED && !this->authenticated_) {
    this->authenticate_();
  }
}

void IGrillClient::dump_config() {
  ESP_LOGCONFIG(TAG, "iGrill Client:");
  ESP_LOGCONFIG(TAG, "  Use Metric: %s", this->use_metric_ ? "YES" : "NO");
  LOG_SENSOR("  ", "Probe 1", this->probe1_sensor_);
  LOG_SENSOR("  ", "Probe 2", this->probe2_sensor_);
  LOG_SENSOR("  ", "Probe 3", this->probe3_sensor_);
  LOG_SENSOR("  ", "Probe 4", this->probe4_sensor_);
  LOG_SENSOR("  ", "Battery", this->battery_sensor_);
  LOG_SENSOR("  ", "Propane", this->propane_sensor_);
}

void IGrillClient::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                       esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT:
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(TAG, "Connected to iGrill device");
        this->authenticated_ = false;
      }
      break;

    case ESP_GATTC_DISCONNECT_EVT:
      ESP_LOGW(TAG, "Disconnected from iGrill device");
      this->authenticated_ = false;
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT:
      ESP_LOGD(TAG, "Service discovery complete");
      this->authenticate_();
      break;

    case ESP_GATTC_NOTIFY_EVT:
      ESP_LOGD(TAG, "Notification received from handle 0x%04x", param->notify.handle);
      // Handle notifications from probes, battery, propane
      if (param->notify.handle == this->probe1_handle_ && this->probe1_sensor_) {
        float temp = this->parse_temperature_(param->notify.value, param->notify.value_len);
        if (temp != INVALID_TEMPERATURE) {
          this->probe1_sensor_->publish_state(temp);
        }
      }
      // TODO: Add similar handling for other probes, battery, propane
      break;

    default:
      break;
  }
}

void IGrillClient::authenticate_() {
  if (this->authenticated_) {
    return;
  }

  ESP_LOGI(TAG, "Authenticating with iGrill device...");

  // TODO: Implement full authentication using igrill_auth
  // For now, just mark as authenticated to allow testing
  this->authenticated_ = true;

  ESP_LOGI(TAG, "Authentication placeholder complete");
  this->subscribe_to_characteristics_();
}

void IGrillClient::subscribe_to_characteristics_() {
  ESP_LOGD(TAG, "Subscribing to characteristics...");

  // TODO: Get characteristic handles using parent()->get_characteristic()
  // TODO: Subscribe to notifications for probes, battery, propane

  ESP_LOGD(TAG, "Characteristic subscription placeholder complete");
}

float IGrillClient::parse_temperature_(const uint8_t *data, uint16_t length) {
  if (length < 2) {
    return INVALID_TEMPERATURE;
  }

  // Check for disconnected probe (0xF8 in byte 1)
  if (data[1] == 0xF8) {
    return INVALID_TEMPERATURE;
  }

  // Parse little-endian temperature
  int16_t raw_temp = (data[1] << 8) | data[0];

  // Convert to degrees (Celsius by default from iGrill)
  float temp_celsius = raw_temp;

  // Convert to Fahrenheit if needed
  if (!this->use_metric_) {
    return (temp_celsius * 9.0f / 5.0f) + 32.0f;
  }

  return temp_celsius;
}

uint8_t IGrillClient::parse_battery_(const uint8_t *data, uint16_t length) {
  if (length < 1) {
    return 0;
  }
  return data[0];
}

uint8_t IGrillClient::parse_propane_(const uint8_t *data, uint16_t length) {
  if (length < 1) {
    return 0;
  }
  // Propane is reported as 0-4, multiply by 25 for percentage
  return data[0] * 25;
}

}  // namespace igrill_client
}  // namespace esphome
