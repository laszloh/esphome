import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@linkedupbits"]
DEPENDENCIES = ["i2c"]
MULTI_CONF = True

CONF_SY6974_ID = "sy6974_id"
CONF_ENABLE_STATUS_LED = "enable_status_led"
CONF_INPUT_CURRENT_LIMIT = "input_current_limit"
CONF_CHARGE_VOLTAGE = "charge_voltage"
CONF_CHARGE_CURRENT = "charge_current"
CONF_PRECHARGE_CURRENT = "precharge_current"
CONF_CHARGE_ENABLED = "charge_enabled"
CONF_JEITA_ENABLED = "jeita_vset_enabled"
CONF_WATCHDOG_TIMEOUT = "watchdog_timeout"
CONF_CHARGER_CURRENT = "charger_current"
CONF_OVERRIDE_INPUT_CURRENT_LIMIT = "override_input_limit"
CONF_VINDPM_VOLTAGE = "vindpm_voltage"
CONF_VINDPM_DYNAMIC = "dynamic_vindpm_voltage"

# Map watchdog timeout (in seconds) to REG_05[5:4] register bits.
# 0 seconds (or "never") disables the watchdog.
WATCHDOG_TIMEOUTS = {
    0: 0x00,
    40: 0x10,
    80: 0x20,
    160: 0x30,
}

def validate_watchdog_timeout(value):
    if isinstance(value, str) and value.lower() == "never":
        return WATCHDOG_TIMEOUTS[0]
    period = cv.positive_time_period_seconds(value)
    seconds = int(period.total_seconds)
    if seconds not in WATCHDOG_TIMEOUTS:
        raise cv.Invalid(
            f"Watchdog timeout must be one of {list(WATCHDOG_TIMEOUTS)} seconds "
            "or 'never' to disable the watchdog"
        )
    return WATCHDOG_TIMEOUTS[seconds]


DYNAMIC_VINDPM = {
    "disabled": 0x00,
    "0mV": 0x00,
    "200mV": 0x01,
    "250mV": 0x02,
    "300mV": 0x03,
}
def validate_dynamic_vindpm(value):
    if not isinstance(value, str) or value not in DYNAMIC_VINDPM:
        raise cv.Invalid(
            f"Dynamic VINDPM scaling must be one of {list(DYNAMIC_VINDPM)} values"
        )
    return DYNAMIC_VINDPM[value]


def validate_100mv_steps(value):
    if value % 100 != 0:
        raise cv.Invalid(f"VINDPM voltage must be in steps of 100mV. {value}mV is not valid.")
    return value


def validate_charger_current_logic(config):
    """Schema-level validator to set the override flag based on charger_current."""
    # Ensure the key exists (it will, if a default is provided in the schema)
    if CONF_INPUT_CURRENT_LIMIT in config:
        value = config[CONF_INPUT_CURRENT_LIMIT]
        if value == "auto":
            config[CONF_OVERRIDE_INPUT_CURRENT_LIMIT] = False
        else:
            config[CONF_OVERRIDE_INPUT_CURRENT_LIMIT] = True
    return config

sy6974_ns = cg.esphome_ns.namespace("sy6974")
SY6974Component = sy6974_ns.class_(
    "SY6974Component", cg.PollingComponent, i2c.I2CDevice
)
SY6974Listener = sy6974_ns.class_("SY6974Listener")

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SY6974Component),
            cv.Optional(CONF_ENABLE_STATUS_LED, default=True): cv.boolean,
            cv.Optional(CONF_INPUT_CURRENT_LIMIT, default="auto"): cv.Any(
                cv.one_of("auto", lower=True),
                cv.int_range(min=100, max=3200),
            ),
            cv.Optional(CONF_CHARGE_VOLTAGE, default=4208): cv.int_range(
                min=3840, max=4608
            ),
            cv.Optional(CONF_CHARGE_CURRENT, default=2048): cv.int_range(
                min=0, max=5056
            ),
            cv.Optional(CONF_PRECHARGE_CURRENT, default=128): cv.int_range(
                min=64, max=1024
            ),
            cv.Optional(CONF_CHARGE_ENABLED, default=True): cv.boolean,
            cv.Optional(CONF_JEITA_ENABLED, default=True): cv.boolean,
            cv.Optional(
                CONF_WATCHDOG_TIMEOUT, default="160s"
            ): validate_watchdog_timeout,
            cv.Optional(CONF_VINDPM_VOLTAGE, default=4500): cv.All(
                cv.int_range(min = 3900, max = 5400),
                validate_100mv_steps,
            ),
            cv.Optional(CONF_VINDPM_DYNAMIC, default="disabled"): validate_dynamic_vindpm,
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(i2c.i2c_device_schema(0x6B))
    .add_extra(validate_charger_current_logic)
)


async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_ENABLE_STATUS_LED],
        config[CONF_INPUT_CURRENT_LIMIT],
        config[CONF_CHARGE_VOLTAGE],
        config[CONF_CHARGE_CURRENT],
        config[CONF_PRECHARGE_CURRENT],
        config[CONF_CHARGE_ENABLED],
        config[CONF_JEITA_ENABLED],
        config[CONF_WATCHDOG_TIMEOUT],
        config[CONF_VINDPM_VOLTAGE],
        config[CONF_VINDPM_DYNAMIC],
        config[CONF_OVERRIDE_INPUT_CURRENT_LIMIT],
    )
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
