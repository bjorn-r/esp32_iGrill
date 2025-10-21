#include "igrill_auth.h"
#include "igrill_const.h"
#include "esphome/core/log.h"

namespace esphome {
namespace igrill_client {

static const char *const TAG = "igrill_auth";

bool IgrillAuthenticator::authenticate(ble_client::BLEClient *client) {
  if (client == nullptr) {
    ESP_LOGE(TAG, "BLE client is null");
    return false;
  }

  client_ = client;
  ESP_LOGI(TAG, "Starting iGrill authentication sequence...");

  // Step 1: Write challenge to APP_CHALLENGE
  if (!send_app_challenge_()) {
    ESP_LOGE(TAG, "Failed to write APP_CHALLENGE");
    return false;
  }

  // Note: Steps 2-3 happen asynchronously via BLE events
  // They will be triggered by the gattc_event_handler in igrill_client.cpp

  return true;
}

bool IgrillAuthenticator::send_app_challenge_() {
  ESP_LOGD(TAG, "Writing APP_CHALLENGE (16 bytes of zeros)...");

  // Get the APP_CHALLENGE characteristic handle
  auto *characteristic = client_->get_characteristic(
    esphome::esp32_ble::ESPBTUUID::from_raw(AUTH_SERVICE_UUID),
    esphome::esp32_ble::ESPBTUUID::from_raw(APP_CHALLENGE_UUID)
  );
  if (characteristic == nullptr) {
    ESP_LOGE(TAG, "APP_CHALLENGE characteristic not found");
    return false;
  }

  // Write 16 zeros to APP_CHALLENGE
  auto status = characteristic->write_value((uint8_t*)AUTH_CHALLENGE, sizeof(AUTH_CHALLENGE));

  if (status != ESP_GATT_OK) {
    ESP_LOGE(TAG, "Failed to write APP_CHALLENGE: %d", status);
    return false;
  }

  ESP_LOGD(TAG, "APP_CHALLENGE written successfully");
  return true;
}

bool IgrillAuthenticator::read_device_challenge_() {
  ESP_LOGD(TAG, "Reading DEVICE_CHALLENGE...");

  // Get the DEVICE_CHALLENGE characteristic handle
  auto *characteristic = client_->get_characteristic(
    esphome::esp32_ble::ESPBTUUID::from_raw(AUTH_SERVICE_UUID),
    esphome::esp32_ble::ESPBTUUID::from_raw(DEVICE_CHALLENGE_UUID)
  );
  if (characteristic == nullptr) {
    ESP_LOGE(TAG, "DEVICE_CHALLENGE characteristic not found");
    return false;
  }

  // Read the characteristic using ESP-IDF GATT client API
  auto status = esp_ble_gattc_read_char(
    client_->get_gattc_if(),
    client_->get_conn_id(),
    characteristic->handle,
    ESP_GATT_AUTH_REQ_NONE
  );

  // The actual data will arrive via ESP_GATTC_READ_CHAR_EVT event
  ESP_LOGD(TAG, "DEVICE_CHALLENGE read initiated");
  return true;
}

bool IgrillAuthenticator::send_device_response_() {
  ESP_LOGD(TAG, "Writing DEVICE_RESPONSE (%d bytes)...", 16);

  // Get the DEVICE_RESPONSE characteristic handle
  auto *characteristic = client_->get_characteristic(
    esphome::esp32_ble::ESPBTUUID::from_raw(AUTH_SERVICE_UUID),
    esphome::esp32_ble::ESPBTUUID::from_raw(DEVICE_RESPONSE_UUID)
  );
  if (characteristic == nullptr) {
    ESP_LOGE(TAG, "DEVICE_RESPONSE characteristic not found");
    return false;
  }

  // Write the encrypted challenge back to DEVICE_RESPONSE
  // Since our challenge is all zeros, we can just echo back what we received
  auto status = characteristic->write_value(device_challenge_, 16);

  if (status != ESP_GATT_OK) {
    ESP_LOGE(TAG, "Failed to write DEVICE_RESPONSE: %d", status);
    return false;
  }

  authenticated_ = true;
  ESP_LOGI(TAG, "Authentication sequence completed successfully");
  return true;
}

}  // namespace igrill_client
}  // namespace esphome
