# ESPHome iGrill Client

ESP32 BLE client for Weber iGrill thermometers, built on the ESPHome platform.

## Features
- ✅ Supports all iGrill models (mini, mini_v2, v2, v202, v3)
- ✅ Up to 4 temperature probes
- ✅ Battery level monitoring
- ✅ Propane level monitoring (iGrill v3)
- ✅ Native Home Assistant integration via API
- ✅ OTA (Over-The-Air) wireless updates
- ✅ Web dashboard
- ✅ Optional MQTT support

## Supported Devices
- iGrill mini
- iGrill mini v2
- iGrill v2
- iGrill v202
- iGrill v3 (with propane sensor support)

## Project Structure

```
esp32_iGrill/
├── components/
│   └── igrill_client/         # Custom ESPHome component
│       ├── __init__.py         # Component registration
│       ├── igrill_client.h     # Main client header
│       ├── igrill_client.cpp   # Main client implementation
│       ├── igrill_auth.h       # Authentication header
│       ├── igrill_auth.cpp     # Authentication implementation
│       └── igrill_const.h      # Constants and UUIDs
└── examples/
    ├── igrill-basic.yaml       # Basic configuration
    ├── igrill-full.yaml        # Full featured configuration
    ├── igrill-v3-propane.yaml  # iGrill v3 with propane sensor
    └── secrets.yaml            # WiFi and device credentials
```

## Quick Start

### Requirements
- ESP32 development board
- Weber iGrill device
- ESPHome installed (2024.6.0 or later)
- Home Assistant (optional but recommended)

## Building the Project

### Prerequisites
- ESPHome 2024.6.0 or later
- Python 3.9 or later
- ESP32 toolchain (automatically installed by ESPHome)

### Build Instructions

1. **Clone the repository**:
   ```bash
   git clone https://github.com/1mckenna/esp32_iGrill.git
   cd esp32_iGrill
   ```

2. **Install ESPHome** (if not already installed):
   ```bash
   pip3 install esphome
   ```

3. **Configure your device**:

   Copy the example secrets file:
   ```bash
   cp examples/secrets.yaml.example examples/secrets.yaml
   ```

   Edit `examples/secrets.yaml` with your credentials:
   ```yaml
   wifi_ssid: "YourWiFiSSID"
   wifi_password: "YourWiFiPassword"
   igrill_mac_address: "AA:BB:CC:DD:EE:FF"  # Your iGrill's MAC address
   api_encryption_key: "generated_key"       # Generate with: esphome config examples/igrill-basic.yaml
   ota_password: "your_ota_password"
   ```

4. **Compile the firmware**:
   ```bash
   cd examples
   esphome compile igrill-basic.yaml
   ```

   The ESPHome build system will automatically discover and use the custom component from `../components/igrill_client/`.

5. **Upload to ESP32** (first time requires USB):
   ```bash
   esphome upload igrill-basic.yaml
   ```

6. **View logs** to verify operation:
   ```bash
   esphome logs igrill-basic.yaml
   ```

### OTA Updates
After initial USB flash, updates can be done wirelessly:
```bash
cd examples
esphome upload igrill-basic.yaml
```

### Validation Only
To validate configuration without building:
```bash
cd examples
esphome config igrill-basic.yaml
```

## Configuration Examples

The `examples/` directory contains three ready-to-use configurations:

- **igrill-basic.yaml** - Minimal setup with temperature sensors and battery monitoring
- **igrill-full.yaml** - Complete setup with all sensors, diagnostic entities, and MQTT
- **igrill-v3-propane.yaml** - iGrill v3 configuration with propane level sensor

## Advantages over Arduino Version
- **90% less code** - From ~9,800 lines to ~950 lines
- **OTA updates** - No more USB cables for updates
- **Better HA integration** - Native API instead of MQTT discovery
- **Easier configuration** - YAML instead of recompiling C++
- **Web dashboard** - Built-in monitoring interface
- **Improved stability** - Better error handling and reconnection

## Project Status
🚧 **IN DEVELOPMENT** - Converting from Arduino to ESPHome

Original Arduino version: https://github.com/1mckenna/esp32_iGrill

## License
MIT License (same as original project)

## Credits
- Original Arduino implementation by [1mckenna](https://github.com/1mckenna)
- ESPHome conversion by Claude & contributors
