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

    case ESP_GATTC_WRITE_CHAR_EVT:
      // Track authentication state transitions via write events
      ESP_LOGD(TAG, "Write characteristic event (handle: 0x%04x)", param->write.handle);
      break;

    case ESP_GATTC_READ_CHAR_EVT:
      // Handle read events during authentication
      ESP_LOGD(TAG, "Read characteristic event (handle: 0x%04x)", param->read.handle);
      break;

    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGD(TAG, "Notification received from handle 0x%04x", param->notify.handle);

      // Probe notification handling
      struct HandleSensorPair {
        uint16_t handle;
        sensor::Sensor* sensor;
      };
      HandleSensorPair pairs[] = {
        {this->probe1_handle_, this->probe1_sensor_},
        {this->probe2_handle_, this->probe2_sensor_},
        {this->probe3_handle_, this->probe3_sensor_},
        {this->probe4_handle_, this->probe4_sensor_},
        {this->battery_handle_, this->battery_sensor_},
        {this->propane_handle_, this->propane_sensor_}
      };

      for (const auto& pair : pairs) {
        if (param->notify.handle == pair.handle && pair.sensor) {
          if (pair.handle == this->battery_handle_) {
            // Battery notification
            uint8_t battery_level = this->parse_battery_(param->notify.value, param->notify.value_len);
            pair.sensor->publish_state(battery_level);
          } else if (pair.handle == this->propane_handle_) {
            // Propane notification
            uint8_t propane_level = this->parse_propane_(param->notify.value, param->notify.value_len);
            pair.sensor->publish_state(propane_level);
          } else {
            // Probe temperature notification
            float temp = this->parse_temperature_(param->notify.value, param->notify.value_len);
            if (temp != INVALID_TEMPERATURE) {
              pair.sensor->publish_state(temp);
            }
          }
          break;
        }
      }
      break;
    }

    default:
      break;
  }
}

void IGrillClient::authenticate_() {
  if (this->authenticated_) {
    return;
  }

  ESP_LOGI(TAG, "Authenticating with iGrill device...");

  // Use the authenticator to perform full authentication
  this->authenticator_.reset();
  this->authenticated_ = this->authenticator_.authenticate(this->parent());

  if (this->authenticated_) {
    ESP_LOGI(TAG, "Authentication successful");
    // Discover characteristics after successful authentication
    this->discover_characteristics_();
  } else {
    ESP_LOGW(TAG, "Authentication failed");
  }
}

void IGrillClient::discover_characteristics_() {
  ESP_LOGI(TAG, "Discovering iGrill characteristics...");

  auto *client = this->parent();
  if (!client) {
    ESP_LOGE(TAG, "No BLE client available for characteristic discovery");
    return;
  }

  // Define an array of probe UUIDs for easier iteration
  const char* probe_uuids[] = {PROBE1_UUID, PROBE2_UUID, PROBE3_UUID, PROBE4_UUID};
  uint16_t* probe_handles[] = {&this->probe1_handle_, &this->probe2_handle_,
                                &this->probe3_handle_, &this->probe4_handle_};

  // Discover probe characteristics
  for (size_t i = 0; i < 4; i++) {
    auto probe_handle = client->get_characteristic(
      esphome::esp32_ble::ESPBTUUID::from_raw(IGRILL_SERVICE_UUID),
      esphome::esp32_ble::ESPBTUUID::from_raw(probe_uuids[i])
    );
    *probe_handles[i] = probe_handle ? probe_handle->handle : 0;

    if (probe_handle) {
      ESP_LOGI(TAG, "Found probe %d characteristic (handle: 0x%04x)", i+1, *probe_handles[i]);
    } else {
      ESP_LOGW(TAG, "Probe %d characteristic not found", i+1);
    }
  }

  // Discover battery characteristic
  auto battery_char = client->get_characteristic(
    esphome::esp32_ble::ESPBTUUID::from_raw(BATTERY_SERVICE_UUID),
    esphome::esp32_ble::ESPBTUUID::from_raw(BATTERY_UUID)
  );
  this->battery_handle_ = battery_char ? battery_char->handle : 0;
  if (battery_char) {
    ESP_LOGI(TAG, "Found battery characteristic (handle: 0x%04x)", this->battery_handle_);
  } else {
    ESP_LOGW(TAG, "Battery characteristic not found");
  }

  // Discover propane characteristic
  auto propane_char = client->get_characteristic(
    esphome::esp32_ble::ESPBTUUID::from_raw(IGRILL_SERVICE_UUID),
    esphome::esp32_ble::ESPBTUUID::from_raw(PROPANE_UUID)
  );
  this->propane_handle_ = propane_char ? propane_char->handle : 0;
  if (propane_char) {
    ESP_LOGI(TAG, "Found propane characteristic (handle: 0x%04x)", this->propane_handle_);
  } else {
    ESP_LOGW(TAG, "Propane characteristic not found");
  }

  // After discovery, subscribe to characteristics
  this->subscribe_to_characteristics_();
}

void IGrillClient::subscribe_to_characteristics_() {
  ESP_LOGD(TAG, "Subscribing to iGrill characteristics...");

  auto *client = this->parent();
  if (!client) {
    ESP_LOGE(TAG, "No BLE client available for characteristic subscription");
    return;
  }

  // Define an array of handles and sensors for easier iteration
  struct HandleSensorPair {
    uint16_t handle;
    sensor::Sensor* sensor;
  };

  HandleSensorPair pairs[] = {
    {this->probe1_handle_, this->probe1_sensor_},
    {this->probe2_handle_, this->probe2_sensor_},
    {this->probe3_handle_, this->probe3_sensor_},
    {this->probe4_handle_, this->probe4_sensor_},
    {this->battery_handle_, this->battery_sensor_},
    {this->propane_handle_, this->propane_sensor_}
  };

  // Subscribe to characteristics
  for (const auto& pair : pairs) {
    if (pair.handle != 0 && pair.sensor) {
      auto status = esp_ble_gattc_register_for_notify(
        client->get_gattc_if(),
        client->get_remote_bda(),
        pair.handle
      );
      if (status) {
        ESP_LOGD(TAG, "Successfully subscribed to characteristic handle 0x%04x", pair.handle);
      } else {
        ESP_LOGW(TAG, "Failed to subscribe to characteristic handle 0x%04x", pair.handle);
      }
    }
  }
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
