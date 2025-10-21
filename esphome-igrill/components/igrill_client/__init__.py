import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, sensor
from esphome.const import (
    CONF_BATTERY_LEVEL,
    CONF_ID,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
)

# Define constants not in esphome.const
CONF_USE_METRIC = "use_metric"

# Set AUTO_LOAD dependency to ensure required components are loaded
AUTO_LOAD = ["ble_client", "sensor"]
DEPENDENCIES = ["ble_client"]
CODEOWNERS = ["@bendudson"]

igrill_client_ns = cg.esphome_ns.namespace("igrill_client")
IGrillClient = igrill_client_ns.class_(
    "IGrillClient", ble_client.BLEClientNode, cg.PollingComponent
)

# Configuration Constants
CONF_PROBE1 = "probe1"
CONF_PROBE2 = "probe2"
CONF_PROBE3 = "probe3"
CONF_PROBE4 = "probe4"
CONF_PROPANE = "propane"

# Configuration Schema
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(IGrillClient),

        # Add use_metric configuration option
        cv.Optional(CONF_USE_METRIC, default=False): cv.boolean,

        # Probe sensor schemas with optional metric/imperial support
        cv.Optional(CONF_PROBE1): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PROBE2): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PROBE3): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PROBE4): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_BATTERY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PROPANE): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
).extend(cv.polling_component_schema("60s")).extend(ble_client.BLE_CLIENT_SCHEMA)


async def to_code(config):
    # Create a new instance of the IGrillClient
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Link to parent BLE client using parent_id
    await ble_client.register_ble_node(var, config)

    # Set the use_metric configuration
    cg.add(var.set_use_metric(config[CONF_USE_METRIC]))

    # Add sensor setup logic (same as before)
    if CONF_PROBE1 in config:
        sens = await sensor.new_sensor(config[CONF_PROBE1])
        cg.add(var.set_probe1_sensor(sens))

    if CONF_PROBE2 in config:
        sens = await sensor.new_sensor(config[CONF_PROBE2])
        cg.add(var.set_probe2_sensor(sens))

    if CONF_PROBE3 in config:
        sens = await sensor.new_sensor(config[CONF_PROBE3])
        cg.add(var.set_probe3_sensor(sens))

    if CONF_PROBE4 in config:
        sens = await sensor.new_sensor(config[CONF_PROBE4])
        cg.add(var.set_probe4_sensor(sens))

    if CONF_BATTERY_LEVEL in config:
        sens = await sensor.new_sensor(config[CONF_BATTERY_LEVEL])
        cg.add(var.set_battery_sensor(sens))

    if CONF_PROPANE in config:
        sens = await sensor.new_sensor(config[CONF_PROPANE])
        cg.add(var.set_propane_sensor(sens))
