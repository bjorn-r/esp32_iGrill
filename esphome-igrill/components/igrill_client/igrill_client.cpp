#include "igrill_client.h"
#include "esphome/core/log.h"
#include "esphome/components/ble_client/ble_client_base.h"
#include "igrill_auth.h"
#include "igrill_const.h"

namespace esphome {
namespace igrill_client {

static const char *const TAG = "igrill_client";

IGrillClient::IGrillClient() {
  ESP_LOGD(TAG, "IGrillClient initialized");
}

void IGrillClient::setup() {
  ESP_LOGD(TAG, "IGrillClient setup");
  // Future: Any setup logic like configuring initial state
}

void IGrillClient::update() {
  // Placeholder for polling logic
  // Future: Implement periodic checks or read operations
  ESP_LOGV(TAG, "IGrillClient update called");
}

void IGrillClient::dump_config() {
  ESP_LOGI(TAG, "IGrillClient Configuration:");
  ESP_LOGI(TAG, "  Temperature Units: %s",
           use_metric_ ? "Metric" : "Imperial");

  // Log connection status
  this->dump_connection_status();
}

void IGrillClient::gattc_event_handler(
    esp_gattc_cb_event_t event,
    esp_gattc_cb_param_t *param,
    esp_gatt_if_t gatt_if
) {
  switch (event) {
    case ESP_GATTC_CONNECT_EVT: {
      ESP_LOGI(TAG, "Connected to iGrill device");
      // Initiate authentication
      this->authenticate_();
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGI(TAG, "Disconnected from iGrill device");
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      // Handle temperature notifications
      this->handle_temperature_notification_(
          param->notify.conn_id,
          param->notify.handle,
          param->notify.value,
          param->notify.value_len
      );
      break;
    }
    default:
      break;
  }
}

float IGrillClient::parse_temperature_(const uint8_t *data, uint16_t length) {
  // Temperature parsing logic from reference implementation
  if (length < 2) {
    ESP_LOGW(TAG, "Insufficient data for temperature parsing");
    return INVALID_TEMPERATURE;
  }

  // Check for disconnected probe
  if (data[1] == 0xF8) {
    ESP_LOGD(TAG, "Probe disconnected");
    return INVALID_TEMPERATURE;
  }

  // Parse as little-endian 16-bit temperature
  int16_t raw_temp = (data[1] << 8) | data[0];

  // Convert raw temperature
  // Add scaling and offset from constants.h if needed
  float temperature = raw_temp * TEMP_SCALE + TEMP_OFFSET;

  // Convert to desired units if needed
  if (!use_metric_) {
    // Convert Celsius to Fahrenheit if using Imperial
    temperature = (temperature * 9.0f / 5.0f) + 32.0f;
  }

  ESP_LOGV(TAG, "Parsed temperature: %.2f %c",
           temperature, use_metric_ ? 'C' : 'F');

  return temperature;
}

void IGrillClient::handle_temperature_notification_(
    uint16_t conn_id,
    uint16_t handle,
    const uint8_t *data,
    uint16_t length
) {
  // Placeholder for temperature notification handling
  float temperature = this->parse_temperature_(data, length);

  // Future: Update sensor values or trigger events
  ESP_LOGD(TAG, "Temperature notification received: %.2f", temperature);
}

uint8_t IGrillClient::parse_battery_(const uint8_t *data, uint16_t length) {
  // Battery parsing logic
  if (length < 1) {
    ESP_LOGW(TAG, "Insufficient data for battery parsing");
    return 0;
  }

  // Battery percentage is the first byte, 0-100
  uint8_t battery_level = data[0];

  ESP_LOGV(TAG, "Parsed battery level: %u%%", battery_level);

  // Publish battery sensor if available
  if (battery_sensor_ != nullptr) {
    battery_sensor_->publish_state(battery_level);
  }

  return battery_level;
}

void IGrillClient::read_battery_level_() {
  if (battery_handle_ == 0) {
    ESP_LOGW(TAG, "Battery characteristic handle not set");
    return;
  }

  // Initiate battery level read
  esp_err_t err = esp_ble_gattc_read_characteristic(
      this->parent()->get_gattc_if(),
      this->parent()->get_conn_id(),
      battery_handle_,
      ESP_GATT_AUTH_REQ_NONE
  );

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read battery level: %s", esp_err_to_name(err));
  } else {
    ESP_LOGD(TAG, "Battery level read request initiated");
  }
}

void IGrillClient::detect_model_() {
  // Determine the iGrill model based on available characteristics
  uint16_t probe2_handle = this->get_characteristic_handle(PROBE2_UUID);
  uint16_t probe3_handle = this->get_characteristic_handle(PROBE3_UUID);
  uint16_t probe4_handle = this->get_characteristic_handle(PROBE4_UUID);
  uint16_t propane_handle = this->get_characteristic_handle(PROPANE_UUID);

  // Only PROBE1 exists → Mini variants
  if (probe1_handle_ != 0 && probe2_handle == 0) {
    // Check for propane sensor to differentiate mini models
    if (propane_handle != 0) {
      igrill_model_ = IGrillModel::MINI;
      igrill_model_name_ = "iGrill_mini";
      ESP_LOGI(TAG, "Detected iGrill Mini (with propane)");
    } else {
      igrill_model_ = IGrillModel::MINI_V2;
      igrill_model_name_ = "iGrill_mini_v2";
      ESP_LOGI(TAG, "Detected iGrill Mini V2");
    }
  }
  // ALL 4 probes exist
  else if (probe1_handle_ != 0 && probe2_handle != 0 &&
           probe3_handle != 0 && probe4_handle != 0) {
    // Check for propane sensor
    if (propane_handle != 0) {
      igrill_model_ = IGrillModel::V3;
      igrill_model_name_ = "iGrill_v3";
      ESP_LOGI(TAG, "Detected iGrill V3 (with propane)");
    }
    // If no propane sensor, try to differentiate between v2 and v202
    else {
      // Future: Add more specific differentiation if needed
      igrill_model_ = IGrillModel::V2;
      igrill_model_name_ = "iGrill_v2";
      ESP_LOGI(TAG, "Detected iGrill V2");
    }
  }
  else {
    ESP_LOGW(TAG, "Unrecognized iGrill model configuration");
  }
}

void IGrillClient::authenticate_() {
  // Use IgrillAuthenticator to perform device authentication
  IgrillAuthenticator authenticator;
  bool auth_success = authenticator.authenticate(this);

  if (auth_success) {
    ESP_LOGI(TAG, "iGrill device authentication successful");

    // Detect the model early in the authentication process
    this->detect_model_();

    // Set temperature units after authentication
    this->set_temperature_units();

    // Setup battery, propane, and probe characteristics after authentication
    this->setup_battery_characteristic_();

    // Only setup propane for models that should have it
    if (igrill_model_ == IGrillModel::V3 || igrill_model_ == IGrillModel::MINI) {
      this->setup_propane_characteristic_();
    }

    this->setup_probe_characteristics_();
  } else {
    ESP_LOGE(TAG, "iGrill device authentication failed");
  }
}

void IGrillClient::dump_config() {
  // Existing configuration dump method
  ESP_LOGI(TAG, "IGrillClient Configuration:");
  ESP_LOGI(TAG, "  Temperature Units: %s",
           use_metric_ ? "Metric" : "Imperial");
  ESP_LOGI(TAG, "  Detected Model: %s", igrill_model_name_.c_str());

  // Log connection status
  this->dump_connection_status();
}

void IGrillClient::setup_battery_characteristic_() {
  // Attempt to get battery characteristic and setup notifications
  battery_handle_ = this->get_characteristic_handle(BATTERY_UUID);

  if (battery_handle_ == 0) {
    ESP_LOGW(TAG, "Battery characteristic not found");
    return;
  }

  // Register for battery notifications
  esp_err_t err = esp_ble_gattc_register_for_notify(
      this->parent()->get_gattc_if(),
      this->parent()->get_remote_bda(),
      battery_handle_
  );

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register for battery notifications: %s", esp_err_to_name(err));
    return;
  }

  // Read initial battery level
  this->read_battery_level_();
}

void IGrillClient::setup_probe_characteristics_() {
  // Get probe characteristic handles
  probe1_handle_ = this->get_characteristic_handle(PROBE1_UUID);
  probe2_handle_ = this->get_characteristic_handle(PROBE2_UUID);
  probe3_handle_ = this->get_characteristic_handle(PROBE3_UUID);
  probe4_handle_ = this->get_characteristic_handle(PROBE4_UUID);

  // Always register for Probe1 notifications
  if (probe1_handle_ != 0) {
    esp_err_t err = esp_ble_gattc_register_for_notify(
        this->parent()->get_gattc_if(),
        this->parent()->get_remote_bda(),
        probe1_handle_
    );

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register for Probe1 notifications: %s", esp_err_to_name(err));
    }
  }

  // Only register for additional probe notifications on multi-probe models
  // iGrill mini/mini_v2 only have Probe1
  if (igrill_model_ != IGrillModel::MINI && igrill_model_ != IGrillModel::MINI_V2) {
    for (const auto& handle : {probe2_handle_, probe3_handle_, probe4_handle_}) {
      if (handle != 0) {
        esp_err_t err = esp_ble_gattc_register_for_notify(
            this->parent()->get_gattc_if(),
            this->parent()->get_remote_bda(),
            handle
        );

        if (err != ESP_OK) {
          ESP_LOGE(TAG, "Failed to register for additional probe notifications: %s", esp_err_to_name(err));
        }
      }
    }
  }
}

void IGrillClient::read_probe_temperature_(uint16_t handle, sensor::Sensor *sensor) {
  if (handle == 0 || sensor == nullptr) {
    ESP_LOGW(TAG, "Invalid probe handle or sensor");
    return;
  }

  // Initiate probe temperature read
  esp_err_t err = esp_ble_gattc_read_characteristic(
      this->parent()->get_gattc_if(),
      this->parent()->get_conn_id(),
      handle,
      ESP_GATT_AUTH_REQ_NONE
  );

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read probe temperature: %s", esp_err_to_name(err));
  } else {
    ESP_LOGD(TAG, "Probe temperature read request initiated");
  }
}

void IGrillClient::setup_propane_characteristic_() {
  // Attempt to get propane characteristic and setup notifications
  propane_handle_ = this->get_characteristic_handle(PROPANE_UUID);

  if (propane_handle_ == 0) {
    ESP_LOGW(TAG, "Propane characteristic not found");
    return;
  }

  // Register for propane notifications
  esp_err_t err = esp_ble_gattc_register_for_notify(
      this->parent()->get_gattc_if(),
      this->parent()->get_remote_bda(),
      propane_handle_
  );

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register for propane notifications: %s", esp_err_to_name(err));
    return;
  }

  // Read initial propane level
  this->read_propane_level_();
}

uint8_t IGrillClient::parse_propane_(const uint8_t *data, uint16_t length) {
  // Propane parsing logic
  if (length < 1) {
    ESP_LOGW(TAG, "Insufficient data for propane parsing");
    return 0;
  }

  // Propane percentage is the first byte multiplied by 25
  uint8_t propane_level = data[0] * 25;

  ESP_LOGV(TAG, "Parsed propane level: %u%%", propane_level);

  // Publish propane sensor if available
  if (propane_sensor_ != nullptr) {
    propane_sensor_->publish_state(propane_level);
  }

  return propane_level;
}

void IGrillClient::read_propane_level_() {
  if (propane_handle_ == 0) {
    ESP_LOGW(TAG, "Propane characteristic handle not set");
    return;
  }

  // Initiate propane level read
  esp_err_t err = esp_ble_gattc_read_characteristic(
      this->parent()->get_gattc_if(),
      this->parent()->get_conn_id(),
      propane_handle_,
      ESP_GATT_AUTH_REQ_NONE
  );

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read propane level: %s", esp_err_to_name(err));
  } else {
    ESP_LOGD(TAG, "Propane level read request initiated");
  }
}

void IGrillClient::gattc_event_handler(
    esp_gattc_cb_event_t event,
    esp_gattc_cb_param_t *param,
    esp_gatt_if_t gatt_if
) {
  switch (event) {
    case ESP_GATTC_CONNECT_EVT: {
      ESP_LOGI(TAG, "Connected to iGrill device");
      // Initiate authentication
      this->authenticate_();
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGI(TAG, "Disconnected from iGrill device");
      battery_handle_ = 0;  // Reset battery handle on disconnect
      propane_handle_ = 0;  // Reset propane handle on disconnect
      probe1_handle_ = 0;   // Reset probe handles on disconnect
      probe2_handle_ = 0;
      probe3_handle_ = 0;
      probe4_handle_ = 0;
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      // First check if this is a temperature notification
      this->handle_temperature_notification_(
          param->notify.conn_id,
          param->notify.handle,
          param->notify.value,
          param->notify.value_len
      );

      // Then check if this is a battery notification
      if (param->notify.handle == battery_handle_) {
        this->parse_battery_(param->notify.value, param->notify.value_len);
      }

      // Check if this is a propane notification
      if (param->notify.handle == propane_handle_) {
        this->parse_propane_(param->notify.value, param->notify.value_len);
      }

      // Check if this is a probe notification
      if (param->notify.handle == probe1_handle_ && probe1_sensor_ != nullptr) {
        float temp = this->parse_temperature_(param->notify.value, param->notify.value_len);
        probe1_sensor_->publish_state(temp);
      }
      if (param->notify.handle == probe2_handle_ && probe2_sensor_ != nullptr) {
        float temp = this->parse_temperature_(param->notify.value, param->notify.value_len);
        probe2_sensor_->publish_state(temp);
      }
      if (param->notify.handle == probe3_handle_ && probe3_sensor_ != nullptr) {
        float temp = this->parse_temperature_(param->notify.value, param->notify.value_len);
        probe3_sensor_->publish_state(temp);
      }
      if (param->notify.handle == probe4_handle_ && probe4_sensor_ != nullptr) {
        float temp = this->parse_temperature_(param->notify.value, param->notify.value_len);
        probe4_sensor_->publish_state(temp);
      }
      break;
    }
    case ESP_GATTC_READ_CHAR_EVT: {
      // Check if this is a battery level read response
      if (param->read.handle == battery_handle_) {
        this->parse_battery_(param->read.value, param->read.value_len);
      }

      // Check if this is a propane level read response
      if (param->read.handle == propane_handle_) {
        this->parse_propane_(param->read.value, param->read.value_len);
      }

      // Check if this is a probe temperature read response
      if (param->read.handle == probe1_handle_ && probe1_sensor_ != nullptr) {
        float temp = this->parse_temperature_(param->read.value, param->read.value_len);
        probe1_sensor_->publish_state(temp);
      }
      if (param->read.handle == probe2_handle_ && probe2_sensor_ != nullptr) {
        float temp = this->parse_temperature_(param->read.value, param->read.value_len);
        probe2_sensor_->publish_state(temp);
      }
      if (param->read.handle == probe3_handle_ && probe3_sensor_ != nullptr) {
        float temp = this->parse_temperature_(param->read.value, param->read.value_len);
        probe3_sensor_->publish_state(temp);
      }
      if (param->read.handle == probe4_handle_ && probe4_sensor_ != nullptr) {
        float temp = this->parse_temperature_(param->read.value, param->read.value_len);
        probe4_sensor_->publish_state(temp);
      }
      break;
    }
    default:
      break;
  }
}

void IGrillClient::set_temperature_units() {
  // Get the temperature units characteristic handle
  uint16_t temp_units_handle = this->get_characteristic_handle(IGRILL_TEMP_UNITS_UUID);

  if (temp_units_handle == 0) {
    ESP_LOGW(TAG, "Temperature units characteristic not found");
    return;
  }

  // Determine units value based on use_metric_
  uint8_t units = use_metric_ ? METRIC_UNITS : IMPERIAL_UNITS;

  // Write the temperature units characteristic
  esp_err_t write_err = esp_ble_gattc_write_characteristic(
    this->parent()->get_gattc_if(),
    this->parent()->get_conn_id(),
    temp_units_handle,
    1,  // Data length
    &units,
    ESP_GATT_WRITE_TYPE_RSP,
    ESP_GATT_AUTH_REQ_NONE
  );

  if (write_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write temperature units: %s", esp_err_to_name(write_err));
    return;
  }

  // Add delay to allow device processing
  vTaskDelay(pdMS_TO_TICKS(500));

  // Read back the temperature units to confirm
  esp_err_t read_err = esp_ble_gattc_read_characteristic(
    this->parent()->get_gattc_if(),
    this->parent()->get_conn_id(),
    temp_units_handle,
    ESP_GATT_AUTH_REQ_NONE
  );

  if (read_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read temperature units: %s", esp_err_to_name(read_err));
    return;
  }

  // Logging for verification
  ESP_LOGI(TAG, "Set temperature units to: %s",
           use_metric_ ? "Metric (Celsius)" : "Imperial (Fahrenheit)");

}  // namespace igrill_client
}  // namespace esphome
