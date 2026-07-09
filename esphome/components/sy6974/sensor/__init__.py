import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_CURRENT,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_MILLIAMP,
)

from .. import CONF_SY6974_ID, SY6974Component, sy6974_ns

DEPENDENCIES = ["sy6974"]

CONF_CHARGE_CURRENT_LIMIT = "charge_current"
CONF_PRECHARGE_CURRENT_LIMIT = "precharge_current"

SY6974CurrentLimitSensor = sy6974_ns.class_("SY6974CurrentLimitSensor", sensor.Sensor)
SY6974PrechargeCurrentSensor = sy6974_ns.class_("SY6974PrechargeCurrentSensor", sensor.Sensor)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SY6974_ID): cv.use_id(SY6974Component),
        cv.Optional(CONF_CHARGE_CURRENT_LIMIT): sensor.sensor_schema(
            SY6974CurrentLimitSensor,
            unit_of_measurement=UNIT_MILLIAMP,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_PRECHARGE_CURRENT_LIMIT): sensor.sensor_schema(
            SY6974PrechargeCurrentSensor,
            unit_of_measurement=UNIT_MILLIAMP,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_SY6974_ID])

    if charge_current_limit_config := config.get(CONF_CHARGE_CURRENT_LIMIT):
        sens = await sensor.new_sensor(charge_current_limit_config)
        cg.add(parent.add_listener(sens))

    if precharge_current_config := config.get(CONF_PRECHARGE_CURRENT_LIMIT):
        sens = await sensor.new_sensor(precharge_current_config)
        cg.add(parent.add_listener(sens))
