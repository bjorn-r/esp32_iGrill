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
  // Check for authentication timeout
  if (this->auth_state_ != AUTH_IDLE && this->auth_state_ != AUTH_AUTHENTICATED) {
    uint32_t elapsed = millis() - this->auth_start_time_;
    if (elapsed > this->auth_timeout_ms_) {
      ESP_LOGW(TAG, "Authentication timeout after %d ms (state: %d)", elapsed, this->auth_state_);
      this->auth_state_ = AUTH_FAILED;
      // Will be reset to AUTH_IDLE on next update cycle or disconnect
    }
  }

  // Polling update - check connection status and initiate auth if needed
  if (this->node_state == esp32_ble_tracker::ClientState::ESTABLISHED &&
      this->auth_state_ == AUTH_IDLE) {
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
      this->auth_state_ = AUTH_IDLE;  // Reset authentication state (including AUTH_FAILED) for reconnection
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT:
      ESP_LOGD(TAG, "Service discovery complete");
      this->authenticate_();
      break;

    case ESP_GATTC_WRITE_CHAR_EVT:
      ESP_LOGD(TAG, "Write characteristic event (handle: 0x%04x, status: %d)", param->write.handle, param->write.status);

      // Check for write failures during authentication
      if (param->write.status != ESP_GATT_OK) {
        if (this->auth_state_ == AUTH_CHALLENGE_SENT || this->auth_state_ == AUTH_RESPONSE_SENT) {
          ESP_LOGE(TAG, "Authentication write failed at state %d with status: %d", this->auth_state_, param->write.status);
          this->auth_state_ = AUTH_FAILED;
        }
        break;
      }

      // Handle authentication write completion
      if (this->auth_state_ == AUTH_CHALLENGE_SENT) {
        ESP_LOGD(TAG, "APP_CHALLENGE write completed, reading DEVICE_CHALLENGE...");
        this->auth_state_ = AUTH_WAITING_DEVICE_CHALLENGE;
        this->authenticator_.read_device_challenge_();
      } else if (this->auth_state_ == AUTH_RESPONSE_SENT) {
        ESP_LOGI(TAG, "Authentication completed successfully");
        this->auth_state_ = AUTH_AUTHENTICATED;
        this->authenticated_ = true;
        this->auth_start_time_ = 0;  // Reset timer
        this->discover_characteristics_();
      }
      break;

    case ESP_GATTC_READ_CHAR_EVT:
      ESP_LOGD(TAG, "Read characteristic event (handle: 0x%04x, status: %d)", param->read.handle, param->read.status);

      // Check for read failures during authentication
      if (param->read.status != ESP_GATT_OK) {
        if (this->auth_state_ == AUTH_WAITING_DEVICE_CHALLENGE) {
          ESP_LOGE(TAG, "Authentication read failed at state %d with status: %d", this->auth_state_, param->read.status);
          this->auth_state_ = AUTH_FAILED;
        }
        break;
      }

      // Handle authentication read completion
      if (this->auth_state_ == AUTH_WAITING_DEVICE_CHALLENGE) {
        ESP_LOGD(TAG, "DEVICE_CHALLENGE read completed, sending DEVICE_RESPONSE...");
        this->auth_state_ = AUTH_RESPONSE_SENT;
        this->authenticator_.handle_device_challenge_read(param->read.value, param->read.value_len);
        this->authenticator_.send_device_response_();
      }
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
  if (this->auth_state_ != AUTH_IDLE) {
    return;  // Already authenticating or authenticated
  }
  ESP_LOGI(TAG, "Starting iGrill authentication...");
  this->authenticator_.reset();
  this->auth_state_ = AUTH_CHALLENGE_SENT;
  this->auth_start_time_ = millis();  // Start timeout timer
  this->authenticator_.authenticate(this->parent());
  // Authentication continues asynchronously via gattc_event_handler
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
      if (status == ESP_GATT_OK) {
        ESP_LOGD(TAG, "Successfully registered for notifications on handle 0x%04x", pair.handle);

        // Enable notifications by writing 0x0001 to CCCD
        // CCCD is typically at characteristic handle + 1
        uint16_t notify_enable = 0x0001;
        uint16_t cccd_handle = pair.handle + 1;

        auto write_status = esp_ble_gattc_write_char_descr(
          client->get_gattc_if(),
          client->get_conn_id(),
          cccd_handle,
          sizeof(notify_enable),
          (uint8_t*)&notify_enable,
          ESP_GATT_WRITE_TYPE_RSP,
          ESP_GATT_AUTH_REQ_NONE
        );

        if (write_status == ESP_GATT_OK) {
          ESP_LOGD(TAG, "Successfully enabled notifications via CCCD for handle 0x%04x", pair.handle);
        } else {
          ESP_LOGW(TAG, "Failed to write CCCD for handle 0x%04x (error: %d)", pair.handle, write_status);
        }
      } else {
        ESP_LOGW(TAG, "Failed to subscribe to characteristic handle 0x%04x (error: %d)", pair.handle, status);
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

  // iGrill sends temperature in tenths of degrees Celsius
  // Raw value must be divided by 10 to get actual temperature
  // TODO: Verify with real hardware - adjust scaling if needed
  // Example: raw_temp = 250 -> temp_celsius = 25.0°C
  float temp_celsius = raw_temp / TEMP_SCALE_FACTOR;

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
