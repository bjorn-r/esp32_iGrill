#pragma once

#include "esphome/core/log.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble/ble_uuid.h"

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
  bool authenticate(ble_client::BLEClient *client);
  bool is_authenticated() const { return authenticated_; }
  void reset() { authenticated_ = false; }

 private:
  ble_client::BLEClient *client_{nullptr};
  bool authenticated_{false};
  uint8_t device_challenge_[16]{};

  bool send_app_challenge_();
  bool read_device_challenge_();
  bool send_device_response_();
};

}  // namespace igrill_client
}  // namespace esphome
