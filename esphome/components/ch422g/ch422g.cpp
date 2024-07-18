#include "CH422G.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ch422g {

// for 16 bit expanders, these addresses will be doubled.
constexpr uint8_t INPUT_REG = 0x26;
constexpr uint8_t OUTPUT_REG = 0x38;
constexpr uint8_t CONFIG_REG = 0x24;

static const char *const TAG = "ch422g";

void CH422GComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CH422G...");
  this->reg_width_ = (this->pin_count_ + 7) / 8;
  // Test to see if device exists
  if (!this->read_inputs_()) {
    ESP_LOGE(TAG, "CH422G not detected at 0x%02X", this->address_);
    this->mark_failed();
    return;
  }

  // All inputs at initialization
  this->config_value_ = 0x01;
  // Invert mask as the part sees a 1 as an input
  this->write_register_(CONFIG_REG, ~this->config_value_);
  // All outputs low
  this->output_mask_ = 0;
  this->write_register_(OUTPUT_REG, this->output_mask_);
  // Read the inputs
  this->read_inputs_();
  ESP_LOGD(TAG, "Initialization complete. Warning: %d, Error: %d", this->status_has_warning(),
           this->status_has_error());
}

void CH422GComponent::loop() {
  // The read_inputs_() method will cache the input values from the chip.
  this->read_inputs_();
  // Clear all the previously read flags.
  this->was_previously_read_ = 0x00;
}

void CH422GComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "CH422G:");
  ESP_LOGCONFIG(TAG, "  I/O Pins: %d", this->pin_count_);
  LOG_I2C_DEVICE(this)
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with CH422G failed!");
  }
}

bool CH422GComponent::digital_read(uint8_t pin) {
  // Note: We want to try and avoid doing any I2C bus read transactions here
  // to conserve I2C bus bandwidth. So what we do is check to see if we
  // have seen a read during the time esphome is running this loop. If we have,
  // we do an I2C bus transaction to get the latest value. If we haven't
  // we return a cached value which was read at the time loop() was called.
  if (this->was_previously_read_ & (1 << pin))
    this->read_inputs_();  // Force a read of a new value
  // Indicate we saw a read request for this pin in case a
  // read happens later in the same loop.
  this->was_previously_read_ |= (1 << pin);
  return this->input_mask_ & (1 << pin);
}

void CH422GComponent::digital_write(uint8_t pin, bool value) {
  if (value) {
    this->output_mask_ |= (1 << pin);
  } else {
    this->output_mask_ &= ~(1 << pin);
  }
  this->write_register_(OUTPUT_REG, this->output_mask_);
}

bool CH422GComponent::read_inputs_() {
  uint8_t inputs;

  if (this->is_failed()) {
    ESP_LOGD(TAG, "Device marked failed");
    return false;
  }

  this->address_ = INPUT_REG;
  if ((this->last_error_ = this->read(&inputs, 1)) != esphome::i2c::ERROR_OK) {
    this->status_set_warning();
    ESP_LOGE(TAG, "read_register_(): I2C I/O error: %d", (int) this->last_error_);
    return false;
  }
  this->status_clear_warning();
  this->input_mask_ = inputs;
  return true;
}

bool CH422GComponent::write_register_(uint8_t reg, uint8_t value) {
  uint8_t output;
  output = value;
  this->address_ = reg;
  if ((this->last_error_ = this->write(&output, 1, true)) != esphome::i2c::ERROR_OK) {
    this->status_set_warning();
    ESP_LOGE(TAG, "write_register_(): I2C I/O error: %d", (int) this->last_error_);
    return false;
  }

  this->status_clear_warning();
  return true;
}

float CH422GComponent::get_setup_priority() const { return setup_priority::IO; }

// Run our loop() method very early in the loop, so that we cache read values before
// before other components call our digital_read() method.
float CH422GComponent::get_loop_priority() const { return 9.0f; }  // Just after WIFI

void CH422GGPIOPin::setup() { pin_mode(flags_); }
void CH422GGPIOPin::pin_mode(gpio::Flags flags) {  }
bool CH422GGPIOPin::digital_read() { return this->parent_->digital_read(this->pin_) != this->inverted_; }
void CH422GGPIOPin::digital_write(bool value) { this->parent_->digital_write(this->pin_, value != this->inverted_); }
std::string CH422GGPIOPin::dump_summary() const {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%u via CH422G", pin_);
  return buffer;
}

}  // namespace CH422G
}  // namespace esphome
