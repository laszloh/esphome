import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import i2c
from esphome.const import (
    CONF_ID,
    CONF_INPUT,
    CONF_NUMBER,
    CONF_MODE,
    CONF_INVERTED,
    CONF_OUTPUT,
)

CODEOWNERS = ["@laszloh"]
DEPENDENCIES = ["i2c"]
MULTI_CONF = True
CONF_PIN_COUNT = "pin_count"
CH422G_ns = cg.esphome_ns.namespace("ch422g")

CH422GComponent = CH422G_ns.class_("CH422GComponent", cg.Component, i2c.I2CDevice)
CH422GGPIOPin = CH422G_ns.class_(
    "CH422GGPIOPin", cg.GPIOPin, cg.Parented.template(CH422GComponent)
)

CONF_CH422G = "CH422G"
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(CH422GComponent),
            cv.Optional(CONF_PIN_COUNT, default=8): cv.one_of(4, 8, 16),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(
        i2c.i2c_device_schema(0x24)
    )
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_pin_count(config[CONF_PIN_COUNT]))
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)


def validate_mode(value):
    if not (value[CONF_OUTPUT]):
        raise cv.Invalid("Mode must be output")
    return value


CH422G_PIN_SCHEMA = pins.gpio_base_schema(
    CH422GGPIOPin,
    cv.int_range(min=0, max=7),
    modes=[CONF_OUTPUT],
    mode_validator=validate_mode,
).extend(
    {
        cv.Required(CONF_CH422G): cv.use_id(CH422GComponent),
    }
)


def CH422G_pin_final_validate(pin_config, parent_config):
    count = parent_config[CONF_PIN_COUNT]
    if pin_config[CONF_NUMBER] >= count:
        raise cv.Invalid(f"Pin number must be in range 0-{count - 1}")


@pins.PIN_SCHEMA_REGISTRY.register(
    CONF_CH422G, CH422G_PIN_SCHEMA, CH422G_pin_final_validate
)
async def CH422G_pin_to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    parent = await cg.get_variable(config[CONF_CH422G])

    cg.add(var.set_parent(parent))

    num = config[CONF_NUMBER]
    cg.add(var.set_pin(num))
    cg.add(var.set_inverted(config[CONF_INVERTED]))
    cg.add(var.set_flags(pins.gpio_flags_expr(config[CONF_MODE])))
    return var
