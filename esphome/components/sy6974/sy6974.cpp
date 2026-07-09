#include "sy6974.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::sy6974 {

static const char *const TAG = "sy6974";

bool SY6974Component::read_all_registers_() {
  // Read all registers from 0x00 to 0x14 in one transaction (12 bytes)
  // This includes unused registers 0x0F, 0x10 for performance
  if (!this->read_bytes(SY6974_REG_INPUT_CURRENT_LIMIT, this->data_.registers, 12)) {
    ESP_LOGW(TAG, "Failed to read registers 0x00-0x0B");
    return false;
  }

  return true;
}

bool SY6974Component::write_register_(uint8_t reg, uint8_t value) {
  if (!this->write_byte(reg, value)) {
    ESP_LOGW(TAG, "Failed to write register 0x%02X", reg);
    return false;
  }
  return true;
}

bool SY6974Component::update_register_(uint8_t reg, uint8_t mask, uint8_t value) {
  uint8_t reg_value;
  if (!this->read_byte(reg, &reg_value)) {
    ESP_LOGW(TAG, "Failed to read register 0x%02X for update", reg);
    return false;
  }
  reg_value = (reg_value & ~mask) | (value & mask);
  return this->write_register_(reg, reg_value);
}

void SY6974Component::setup() {
  ESP_LOGV(TAG, "Setting up SY6974...");

  // Try to read chip ID
  uint8_t reg_value;
  if (!this->read_byte(SY6974_REG_DEVICE_ID, &reg_value)) {
    ESP_LOGE(TAG, "Failed to communicate with SY6974");
    this->mark_failed();
    return;
  }

  uint8_t chip_id = reg_value & 0x03;
  if (chip_id != 0x00) {
    ESP_LOGW(TAG, "Unexpected chip ID: 0x%02X (expected 0x00)", chip_id);
  }

  // Apply configuration options (all have defaults now)
  ESP_LOGV(TAG, "Setting LED enabled to %s", ONOFF(this->led_enabled_));
  this->set_led_enabled(this->led_enabled_);

  ESP_LOGV(TAG, "Setting charge voltage to %u mV", this->charge_voltage_);
  this->set_charge_target_voltage(this->charge_voltage_);

  ESP_LOGV(TAG, "Setting charge current to %u mA", this->charge_current_);
  this->set_charge_current(this->charge_current_);

  ESP_LOGV(TAG, "Setting precharge current to %u mA", this->precharge_current_);
  this->set_precharge_current(this->precharge_current_);

  ESP_LOGV(TAG, "Setting charge enabled to %s", ONOFF(this->charge_enabled_));
  this->set_charge_enabled(this->charge_enabled_);

  ESP_LOGV(TAG, "Setting JEITA voltage reduction enabled to %s", ONOFF(this->jeita_vset_enabled_));
  this->set_jeita_vset_enabled(this->jeita_vset_enabled_);

  ESP_LOGV(TAG, "Setting watchdog timeout register bits to 0x%02X", this->watchdog_timeout_);
  this->set_watchdog_timeout(this->watchdog_timeout_);

  if(this->override_input_limit_) {
    ESP_LOGW(TAG, "Input current of charger will be overriden!");
  
    ESP_LOGV(TAG, "Setting input current limit to %u mA", this->input_current_limit_);
    this->set_input_current_limit(this->input_current_limit_);
  }

  ESP_LOGV(TAG, "Setting VINDPM static limit to %u mA", this->vindpm_voltage_);
  this->set_vindpm_voltage(this->vindpm_voltage_);

  ESP_LOGV(TAG, "Setting dynamic VINDPM register bit to %02X", this->vindpm_dynamic_);
  this->set_vindpm_dynamic_voltage(this->vindpm_dynamic_reg_);

  ESP_LOGV(TAG, "SY6974 initialized successfully");
}

void SY6974Component::dump_config() {
  // REG_05[5:4] bits (0x00/0x10/0x20/0x30) mapped back to seconds for logging.
  static const uint16_t WATCHDOG_TIMEOUT_SECONDS[] = {0, 40, 80, 160};
  static const uint16_t VINDPM_DYNAMIC_VOLATGES[] = {0, 200, 250, 300};
  ESP_LOGCONFIG(TAG,
                "SY6974:\n"
                "  LED Enabled: %s\n"
                "  Input Current Limit: %u mA\n"
                "  Input Current Limit override: %s\n"
                "  Charge Voltage: %u mV\n"
                "  Charge Current: %u mA\n"
                "  Precharge Current: %u mA\n"
                "  Charge Enabled: %s\n"
                "  JEITA VSET Enabled: %s\n"
                "  Watchdog Timeout: %u s\n"
                "  VINDPM Voltage: %u mV\n"
                "  VINDPM dynamic voltage: %u mV\n",
                ONOFF(this->led_enabled_), this->input_current_limit_, ONOFF(this->override_input_limit_), 
                this->charge_voltage_, this->charge_current_, this->precharge_current_, 
                ONOFF(this->charge_enabled_), ONOFF(this->jeita_vset_enabled_),
                WATCHDOG_TIMEOUT_SECONDS[this->watchdog_timeout_ >> 4], this->vindpm_voltage_, 
                VINDPM_DYNAMIC_VOLATGES[this->vindpm_dynamic_reg_]);
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with SY6974 failed!");
  }
}

void SY6974Component::update() {
  if (this->is_failed()) {
    return;
  }

  // Read all registers in one transaction
  if (!this->read_all_registers_()) {
    ESP_LOGW(TAG, "Failed to read registers during update");
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();

  // kick watchdog
  this->update_register_(SY6974_REG_SYS_CONTROL, WATCHDOG_RESET_MASK, WATCHDOG_RESET_MASK);

  // override charger current
  if(this->override_input_limit_) {
    set_input_current_limit(this->input_current_limit_);
  }

  // Notify all listeners with the new data
  for (auto *listener : this->listeners_) {
    listener->on_data(this->data_);
  }
}

void SY6974Component::set_input_current_limit(uint16_t milliamps) {
  if (this->is_failed())
    return;

  input_current_limit_ = milliamps;
  if (milliamps < INPUT_CURRENT_MIN) {
    milliamps = INPUT_CURRENT_MIN;
  }

  uint8_t val = (milliamps - INPUT_CURRENT_MIN) / INPUT_CURRENT_STEP;
  if (val > INPUT_CURRENT_STEP_MAX) {
    val = INPUT_CURRENT_STEP_MAX;
  }

  this->update_register_(SY6974_REG_INPUT_CURRENT_LIMIT, INPUT_CURRENT_STEP_MASK, val);
}

void SY6974Component::set_charge_target_voltage(uint16_t millivolts) {
  if (this->is_failed())
    return;

  charge_voltage_ = millivolts;
  uint8_t val = CHG_VOLTAGE_SPECIAL_VAL;
  if(millivolts != CHG_VOLTAGE_SPECIAL) {
    if (millivolts < CHG_VOLTAGE_BASE) {
      millivolts = CHG_VOLTAGE_BASE;
    }

    val = (millivolts - CHG_VOLTAGE_BASE) / CHG_VOLTAGE_STEP;
    if (val > CHG_VOLTAGE_STEP_MAX) {
      val = CHG_VOLTAGE_STEP_MAX;
    }
  }

  this->update_register_(SY6974_REG_CHARGE_VOLTAGE, CHG_VOLTAGE_STEP_MASK, val << 3);
}

void SY6974Component::set_precharge_current(uint16_t milliamps) {
  if (this->is_failed())
    return;

  precharge_current_ = milliamps;
  if (milliamps < PRE_CHG_BASE_MA) {
    milliamps = PRE_CHG_BASE_MA;
  }

  uint8_t val = (milliamps - PRE_CHG_BASE_MA) / PRE_CHG_STEP_MA;
  if (val > PRE_CHG_STEP_MAX) {
    val = PRE_CHG_STEP_MAX;
  }

  this->update_register_(SY6974_REG_PRECHARGE_CURRENT, PRE_CHG_STEP_MASK, val << 4);
}

void SY6974Component::set_charge_current(uint16_t milliamps) {
  if (this->is_failed())
    return;

  charge_current_ = milliamps;
  uint8_t val = milliamps / CHG_CURRENT_STEP_MA;
  if (val > CHG_CURRENT_STEP_MAX) {
    val = CHG_CURRENT_STEP_MAX;
  }

  this->update_register_(SY6974_REG_CHARGE_CURRENT, CHG_CURRENT_STEP_MASK, val);
}

void SY6974Component::set_charge_enabled(bool enabled) {
  if (this->is_failed())
    return;

  charge_enabled_ = enabled;
  this->update_register_(SY6974_REG_SYS_CONTROL, 0x10, enabled ? 0x10 : 0x00);
}

void SY6974Component::set_jeita_vset_enabled(bool enabled) {
  if (this->is_failed())
    return;

  jeita_vset_enabled_ = enabled;
  this->update_register_(SY6974_REG_FORCE_DPDM, JEITA_VSET_MASK, enabled ? 0x00 : JEITA_VSET_MASK);
}

void SY6974Component::set_led_enabled(bool enabled) {
  if (this->is_failed())
    return;

  led_enabled_ = enabled;
  // Bit 5:6: 00 = LED enabled, 11 = LED disabled
  this->update_register_(SY6974_REG_INPUT_CURRENT_LIMIT, 0x60, enabled ? 0x00 : 0x60);
}

void SY6974Component::set_watchdog_timeout(uint8_t timeout_bits) {
  if (this->is_failed())
    return;

  watchdog_timeout_ = timeout_bits;
  // REG_05[5:4]: 0x00 = off, 0x10 = 40s, 0x20 = 80s, 0x30 = 160s
  this->update_register_(SY6974_REG_TIMER_CONTROL, WATCHDOG_TIMEOUT_MASK, timeout_bits);
}

void SY6974Component::set_vindpm_voltage(uint16_t vindpm_voltage) {
  if (this->is_failed())
    return;

    this->vindpm_voltage_ = vindpm_voltage;

    if(vindpm_voltage < VINDPM_VOLTAGE_BASE)
      vindpm_voltage = VINDPM_VOLTAGE_BASE;

    uint8_t val = (vindpm_voltage - VINDPM_VOLTAGE_BASE) / VINDPM_VOLATGE_STEP;
    if(val > VINDPM_STEP_MASK) {
      val = VINDPM_STEP_MASK;
    }

    // REG_06[3:0]: VINDPM value in 100mV steps with 3,9V offset
    this->update_register_(SY6974_REG_VINDPM, VINDPM_STEP_MASK, val);

}

void SY6974Component::set_vindpm_dynamic_voltage(uint8_t vindpm_dynamic) {
  if (this->is_failed())
    return;

  this->vindpm_dynamic_reg_ = vindpm_dynamic;
  // REG_07[1:0]: VDPM_BAT_TRACK with 0x00 = disabled, 0x01 = VBAT+200mV, 0x02 = VBAT+250mV, 0x03 =VBAT+300mV
  this->update_register_(SY6974_REG_FORCE_DPDM, VINDPM_DYNAMIC_MASK, vindpm_dynamic);
}

void SY6974Component::set_input_current_limit_override(bool override_input_limit) {
  this->override_input_limit_ = override_input_limit;
}

}  // namespace esphome::sy6974
