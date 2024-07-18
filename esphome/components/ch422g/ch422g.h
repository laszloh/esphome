#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace ch422g {

class CH422GComponent : public Component, public i2c::I2CDevice {
 public:
  CH422GComponent() = default;

  /// Check i2c availability and setup masks
  void setup() override;
  /// Poll for input changes periodically
  void loop() override;
  /// Helper function to read the value of a pin.
  bool digital_read(uint8_t pin);
  /// Helper function to write the value of a pin.
  void digital_write(uint8_t pin, bool value);

  float get_setup_priority() const override;

  float get_loop_priority() const override;

  void dump_config() override;

  void set_pin_count(size_t pin_count) { this->pin_count_ = pin_count; }

 protected:
  bool read_inputs_();

  bool write_register_(uint8_t reg, uint8_t value);

  /// number of bits the expander has
  size_t pin_count_{8};
  /// width of registers
  size_t reg_width_{1};
  /// Configuration value (0x00 .. all pins INPUT, 0x01 .. all pins OUTPUT)
  uint8_t config_value_{0x01};
  /// The mask to write as output state - 1 means HIGH, 0 means LOW
  uint8_t output_mask_{0x00};
  /// The state of the actual input pin states - 1 means HIGH, 0 means LOW
  uint8_t input_mask_{0x00};
  /// Flags to check if read previously during this loop
  uint8_t was_previously_read_ = {0x00};
  /// Storage for last I2C error seen
  esphome::i2c::ErrorCode last_error_;
};

/// Helper class to expose a CH422G pin as an internal input GPIO pin.
class CH422GGPIOPin : public GPIOPin {
 public:
  void setup() override;
  void pin_mode(gpio::Flags flags) override;
  bool digital_read() override;
  void digital_write(bool value) override;
  std::string dump_summary() const override;

  void set_parent(CH422GComponent *parent) { parent_ = parent; }
  void set_pin(uint8_t pin) { pin_ = pin; }
  void set_inverted(bool inverted) { inverted_ = inverted; }
  void set_flags(gpio::Flags flags) { flags_ = flags; }

 protected:
  CH422GComponent *parent_;
  uint8_t pin_;
  bool inverted_;
  gpio::Flags flags_;
};

}  // namespace ch422g
}  // namespace esphome