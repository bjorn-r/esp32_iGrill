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
