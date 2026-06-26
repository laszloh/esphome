#pragma once

#include "epaper_waveshare_bwr.h"

namespace esphome::epaper_spi {

/**
 * DKE DEPG0750RWF86BF 7.5" 800x480 Black/White/Red e-paper.
 *
 * Controller: JD79686 (FITI / "LUT from OTP" panel).
 *
 * Shares the UC8179-family data/refresh/power protocol with
 * EPaperWaveshareBWR (DTM1 0x10 / DTM2 0x13 / DRF 0x12 / POF 0x02 /
 * DSLP 0x07,0xA5), so the buffer layout, transfer_data(), refresh_screen(),
 * power_off() and deep_sleep() are all inherited unchanged.
 *
 * The two differences vs. the Waveshare 7.5" V2:
 *   1. The init sequence is the DKE FITI vendor block (set in the model's
 *      get_init_sequence(), see models/dke86bf.py).
 *   2. This is a LUT-from-OTP panel: it must NOT receive a 0x01 power
 *      setting. The base power_on() sends 0x01 + 0x04; here we override it
 *      to send PON (0x04) only, matching the DKE reference flow (spec 10.2).
 */
class EPaperDke86BF : public EPaperWaveshareBWR {
 public:
  using EPaperWaveshareBWR::EPaperWaveshareBWR;

  bool is_busy() const { return this->state_ != EPaperState::IDLE; }



 protected:

 void power_on() override {
    this->command(0x04);  // PON only; panel uses its OTP power defaults
  }

};

}  // namespace esphome::epaper_spi