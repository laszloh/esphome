#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include <vector>

namespace esphome::sy6974 {

// SY6974 Register addresses with descriptive names
static const uint8_t SY6974_REG_INPUT_CURRENT_LIMIT = 0x00;     // HIZ, LED & Input current limit control
static const uint8_t SY6974_REG_SYS_CONTROL = 0x01;             // System control, PFM, WD, OTG, CHG_EN, SYS_MIN, OTG_BAT
static const uint8_t SY6974_REG_CHARGE_CURRENT = 0x02;          // Fast charge current limit
static const uint8_t SY6974_REG_PRECHARGE_CURRENT = 0x03;       // Pre-charge/termination current
static const uint8_t SY6974_REG_CHARGE_VOLTAGE = 0x04;          // Charge voltage limit
static const uint8_t SY6974_REG_TIMER_CONTROL = 0x05;           // Charge timer and watchdog control
static const uint8_t SY6974_REG_VINDPM = 0x06;                  // Input voltage limit
static const uint8_t SY6974_REG_FORCE_DPDM = 0x07;              // Force DPDM detection
static const uint8_t SY6974_REG_STATUS = 0x08;                  // System status (bus, charge status), read-only
static const uint8_t SY6974_REG_FAULT = 0x09;                   // Fault status (NTC), read-only
static const uint8_t SY6974_REG_VBUS_STATUS = 0x0A;             // VBUS status
static const uint8_t SY6974_REG_DEVICE_ID = 0x0B;               // Part information

// Constants for voltage and current calculations
static const uint8_t WATCHDOG_RESET_MASK = 0x40;      // bits
static const uint8_t WATCHDOG_TIMEOUT_MASK = 0x30;    // bits

static const uint16_t PRE_CHG_BASE_MA = 60;           // mA
static const uint16_t PRE_CHG_STEP_MA = 60;           // mA
static const uint8_t PRE_CHG_STEP_MAX = 0x0C;         // steps
static const uint8_t PRE_CHG_STEP_MASK = 0xF0;        // bits

static const uint16_t CHG_CURRENT_STEP_MA = 60;       // mA
static const uint16_t CHG_CURRENT_STEP_MAX = 50;      // steps
static const uint16_t CHG_CURRENT_STEP_MASK = 0x3F;   // bits

static const uint16_t CHRGR_CURRENT_BASE_MA = 100;    // mA
static const uint16_t CHRGR_CURRENT_STEP_MA = 100;    // mA
static const uint16_t CHGGR_CURRENT_STEP_MAX = 31;    // steps
static const uint16_t CHGGR_CURRENT_STEP_MASK = 0x1F; // bits

static const uint16_t CHG_VOLTAGE_BASE = 3856;        // mV
static const uint16_t CHG_VOLTAGE_STEP = 32;          // mV
static const uint8_t CHG_VOLTAGE_STEP_MAX = 0x18;     // steps
static const uint8_t CHG_VOLTAGE_STEP_MASK = 0xF8;    // bits
static const uint16_t CHG_VOLTAGE_SPECIAL = 4352;     // mV 
static const uint8_t CHG_VOLTAGE_SPECIAL_VAL = 0x0F;  // bit

static const uint16_t INPUT_CURRENT_MIN = 100;        // mA
static const uint16_t INPUT_CURRENT_STEP = 100;       // mA
static const uint8_t INPUT_CURRENT_STEP_MAX = 0x1F;   // steps
static const uint8_t INPUT_CURRENT_STEP_MASK = 0x1F;  // bits

static const uint8_t JEITA_VSET_MASK = 0x10;          // bits

static const uint16_t VINDPM_VOLTAGE_BASE = 3900;     // mV
static const uint16_t VINDPM_VOLATGE_STEP = 100;      // steps
static const uint8_t VINDPM_STEP_MASK = 0x0F;         // bits

static const uint8_t VINDPM_DYNAMIC_MASK = 0x03;

// Bus Status values (REG_08[7:5])
enum BusStatus {
  BUS_STATUS_NO_INPUT = 0,
  BUS_STATUS_USB_SDP = 1,
  BUS_STATUS_USB_CDP = 2,
  BUS_STATUS_USB_DCP = 3,
  BUS_STATUS_HVDCP = 4,
  BUS_STATUS_ADAPTER = 5,
  BUS_STATUS_NO_STD_ADAPTER = 6,
  BUS_STATUS_OTG = 7,
};

// Charge Status values (REG_08[4:3])
enum ChargeStatus {
  CHARGE_STATUS_NOT_CHARGING = 0,
  CHARGE_STATUS_PRE_CHARGE = 1,
  CHARGE_STATUS_FAST_CHARGE = 2,
  CHARGE_STATUS_CHARGE_DONE = 3,
};

// Structure to hold all register data read in one transaction
struct SY6974Data {
  uint8_t registers[12];  // Registers 0x00-0x0B
};

// Listener interface for components that want to receive SY6974 data updates
class SY6974Listener {
 public:
  virtual void on_data(const SY6974Data &data) = 0;
};

class SY6974Component : public PollingComponent, public i2c::I2CDevice {
 public:
  SY6974Component(bool led_enabled, uint16_t input_current_limit, uint16_t charge_voltage, uint16_t charge_current,
                  uint16_t precharge_current, bool charge_enabled, bool jeita_vset_enabled, uint8_t watchdog_timeout,
                  uint16_t vindpm_voltage, uint16_t vindpm_dynamic, bool override_input_limit)
      : led_enabled_(led_enabled),
        input_current_limit_(input_current_limit),
        charge_voltage_(charge_voltage),
        charge_current_(charge_current),
        precharge_current_(precharge_current),
        charge_enabled_(charge_enabled),
        jeita_vset_enabled_(jeita_vset_enabled),
        watchdog_timeout_(watchdog_timeout),
        vindpm_voltage_(vindpm_voltage),
        vindpm_dynamic_reg_(vindpm_dynamic),
        override_input_limit_(override_input_limit) {}
  void setup() override;
  void dump_config() override;
  void update() override;

  // Listener registration
  void add_listener(SY6974Listener *listener) { this->listeners_.push_back(listener); }

  // Configuration methods to be called from lambdas
  void set_input_current_limit(uint16_t milliamps);
  void set_charge_target_voltage(uint16_t millivolts);
  void set_precharge_current(uint16_t milliamps);
  void set_charge_current(uint16_t milliamps);
  void set_charge_enabled(bool enabled);
  void set_jeita_vset_enabled(bool enabled);
  void set_led_enabled(bool enabled);
  void set_watchdog_timeout(uint8_t timeout_bits);
  void set_vindpm_voltage(uint16_t vindpm_voltage);
  void set_vindpm_dynamic_voltage(uint8_t vindpm_dynamic);
  void set_input_current_limit_override(bool override_input_limit);

 protected:
  bool read_all_registers_();
  bool write_register_(uint8_t reg, uint8_t value);
  bool update_register_(uint8_t reg, uint8_t mask, uint8_t value);

  SY6974Data data_{};
  std::vector<SY6974Listener *> listeners_;

  // Configuration values to set during setup()
  bool led_enabled_;
  uint16_t input_current_limit_;
  uint16_t charge_voltage_;
  uint16_t charge_current_;
  uint16_t precharge_current_;
  bool charge_enabled_;
  bool jeita_vset_enabled_;
  uint8_t watchdog_timeout_;  // REG_05[5:4] bits: 0x00=off, 0x10=40s, 0x20=80s, 0x30=160s
  uint16_t vindpm_voltage_;
  uint8_t vindpm_dynamic_reg_;
  bool override_input_limit_;
};

}  // namespace esphome::sy6974
