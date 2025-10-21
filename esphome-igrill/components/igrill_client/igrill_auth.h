#pragma once

#include "esphome/core/log.h"
#include "esphome/components/ble_client/ble_client.h"
#include "igrill_const.h"

namespace esphome {
namespace igrill_client {

/**
 * IgrillAuthenticator handles the iGrill BLE authentication sequence
 */
class IgrillAuthenticator {
 public:
  /**
   * Perform full authentication sequence
   * Returns true if authentication succeeded
   */
  static bool authenticate(
      ble_client::BLEClient *client,
      uint16_t app_challenge_handle,
      uint16_t device_challenge_handle,
      uint16_t device_response_handle
  );

 private:
  static bool write_app_challenge_(ble_client::BLEClient *client, uint16_t handle);
  static bool read_device_challenge_(ble_client::BLEClient *client, uint16_t handle,
                                     uint8_t *buffer, uint16_t *length);
  static bool write_device_response_(ble_client::BLEClient *client, uint16_t handle,
                                     const uint8_t *data, uint16_t length);
};

}  // namespace igrill_client
}  // namespace esphome
