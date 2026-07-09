#pragma once

#include "../sy6974.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome::sy6974 {

template<uint8_t REG, uint8_t MASK, uint16_t BASE, uint16_t STEP>
class CurrentSensor : public SY6974Listener, public sensor::Sensor {
 public:
  void on_data(const SY6974Data &data) override {
    uint8_t val = data.registers[REG] & MASK;
    uint16_t current_ma = BASE + (val * STEP);
    this->publish_state(current_ma);
  }
};

using SY6974CurrentLimitSensor = CurrentSensor<SY6974_REG_INPUT_CURRENT_LIMIT, 0x1F, INPUT_CURRENT_MIN, INPUT_CURRENT_STEP>;

// Precharge current sensor needs special handling (bit shift)
class SY6974PrechargeCurrentSensor : public SY6974Listener, public sensor::Sensor {
 public:
  void on_data(const SY6974Data &data) override {
    uint8_t iprechg = (data.registers[SY6974_REG_PRECHARGE_CURRENT] >> 4) & 0x0F;
    uint16_t iprechg_ma = PRE_CHG_BASE_MA + (iprechg * PRE_CHG_STEP_MA);
    this->publish_state(iprechg_ma);
  }
};

}  // namespace esphome::sy6974
