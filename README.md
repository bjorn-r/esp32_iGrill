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

## Quick Start

### Requirements
- ESP32 development board
- Weber iGrill device
- ESPHome installed
- Home Assistant (optional but recommended)

### Installation
See [Installation Guide](docs/INSTALLATION.md) for detailed setup instructions.

## Building the Project

### Prerequisites
- ESPHome 2024.6.0 or later
- Python 3.9 or later
- ESP32 toolchain (automatically installed by ESPHome)

### Build Instructions

1. **Install ESPHome** (if not already installed):
   ```bash
   pip3 install esphome
   ```

2. **Configure your device** by editing `examples/igrill-basic.yaml` or create your own configuration file

3. **Update secrets** in `examples/secrets.yaml`:
   ```yaml
   wifi_ssid: "YourWiFiSSID"
   wifi_password: "YourWiFiPassword"
   igrill_mac_address: "AA:BB:CC:DD:EE:FF"  # Your iGrill's MAC address
   api_encryption_key: "generated_key"       # Generate with: esphome config igrill-basic.yaml
   ota_password: "your_ota_password"
   ```

4. **Compile the firmware**:
   ```bash
   cd examples
   esphome compile igrill-basic.yaml
   ```

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
esphome upload igrill-basic.yaml
```

### Validation Only
To validate configuration without building:
```bash
esphome config igrill-basic.yaml
```

## Documentation
- [Installation Guide](docs/INSTALLATION.md)
- [Configuration Reference](docs/CONFIGURATION.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)
- [Migration from Arduino](docs/MIGRATION_FROM_ARDUINO.md)

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
