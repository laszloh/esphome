import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import DEVICE_CLASS_CONNECTIVITY, DEVICE_CLASS_POWER

from .. import CONF_SY6974_ID, SY6974Component, sy6974_ns

DEPENDENCIES = ["sy6974"]

CONF_VBUS_CONNECTED = "vbus_connected"
CONF_CHARGING = "charging"
CONF_CHARGE_DONE = "charge_done"

SY6974VbusConnectedBinarySensor = sy6974_ns.class_(
    "SY6974VbusConnectedBinarySensor", binary_sensor.BinarySensor
)
SY6974ChargingBinarySensor = sy6974_ns.class_(
    "SY6974ChargingBinarySensor", binary_sensor.BinarySensor
)
SY6974ChargeDoneBinarySensor = sy6974_ns.class_(
    "SY6974ChargeDoneBinarySensor", binary_sensor.BinarySensor
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SY6974_ID): cv.use_id(SY6974Component),
        cv.Optional(CONF_VBUS_CONNECTED): binary_sensor.binary_sensor_schema(
            SY6974VbusConnectedBinarySensor,
            device_class=DEVICE_CLASS_CONNECTIVITY,
        ),
        cv.Optional(CONF_CHARGING): binary_sensor.binary_sensor_schema(
            SY6974ChargingBinarySensor,
            device_class=DEVICE_CLASS_POWER,
        ),
        cv.Optional(CONF_CHARGE_DONE): binary_sensor.binary_sensor_schema(
            SY6974ChargeDoneBinarySensor,
            device_class=DEVICE_CLASS_POWER,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_SY6974_ID])

    if vbus_connected_config := config.get(CONF_VBUS_CONNECTED):
        sens = await binary_sensor.new_binary_sensor(vbus_connected_config)
        cg.add(parent.add_listener(sens))

    if charging_config := config.get(CONF_CHARGING):
        sens = await binary_sensor.new_binary_sensor(charging_config)
        cg.add(parent.add_listener(sens))

    if charge_done_config := config.get(CONF_CHARGE_DONE):
        sens = await binary_sensor.new_binary_sensor(charge_done_config)
        cg.add(parent.add_listener(sens))
