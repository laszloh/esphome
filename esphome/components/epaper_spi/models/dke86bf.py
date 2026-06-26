"""DKE Black/White/Red e-paper displays using the JD79686 (FITI) controller.

Supported models:
- dke-7.5in-86bf-bwr: 800x480 pixels (DEPG0750RWF86BF, 7.5" BWR)

This is a "LUT from OTP" panel: the waveform lives in on-chip OTP, so the
init sequence sends only the DKE FITI vendor block. Power-on is PON (0x04)
only -- handled by the EPaperDke86BF C++ class, NOT here -- because sending
a 0x01 power setting would override the panel's programmed defaults.
"""

from . import EpaperModel


class Dke86BF(EpaperModel):
    """EpaperModel for DKE DEPG0750RWF86BF (JD79686 / FITI controller)."""

    def __init__(self, name, **defaults):
        super().__init__(name, "EPaperDke86BF", **defaults)

    def get_init_sequence(self, config):
        """DKE FITI init block (spec 10.2 reference flow).

        Note: register 0xF7 is used by the proven GxEPD2 driver for this
        panel; the DKE datasheet reference flow lists 0xE7 in this slot.
        If the panel handshakes BUSY but never refreshes, swap 0xF7 -> 0xE7.
        Do NOT add a 0x00 panel-setting or 0x01 power-setting here: this is
        a LUT-from-OTP panel and must use its OTP defaults.
        """
        return (
            (0x4D, 0x55),
            (0xA6, 0x38),
            (0xA7, 0x2A),
            (0xB6, 0x80),
            (0xB7, 0x00),
            (0xB4, 0x5D),
            (0xF7, 0x02),
            (0x00, 0xCF, 0x09)
        )


# Model: DKE 7.5" BWR -- 800x480, JD79686 controller
Dke86BF(
    "dke-7.5in-86bf-bwr",
    width=800,
    height=480,
    data_rate="10MHz",
    minimum_update_interval="30s",
)