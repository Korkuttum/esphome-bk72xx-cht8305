# esphome-bk72xx-cht8305

ESPHome external component for the **CHT8305** temperature/humidity sensor on
**BK72xx (LibreTiny)** boards such as the CBU module (BK7231N).

ESPHome has no I2C bus implementation for the `bk72xx` platform, so the usual
`i2c:` + sensor combination is not available. This component talks to the
sensor with a small bit-banged (software) I2C driver on two ordinary GPIO pins
and needs no `i2c:` block.

Found on: Tuya Wi-Fi IR remote with a temperature/humidity sensor, CBU module
(BK7231N), sensor on **SDA = P6, SCL = P26**. Other Tuya devices with the same
chip will likely use different pins.

## Usage

```yaml
external_components:
  - source: github://Korkuttum/esphome-bk72xx-cht8305
    components: [cht8305]

sensor:
  - platform: cht8305
    sda_pin: P6
    scl_pin: P26
    update_interval: 60s
    temperature:
      name: "Temperature"
    humidity:
      name: "Humidity"
```

See [`examples/example.yaml`](examples/example.yaml) for a complete config.

## Finding the pins

If you do not know which pins the sensor uses, probe candidate pin pairs for
an ACK at address `0x40` (CHT8305 reads `0x5959` from register `0xFE` and
`0x8305` from register `0xFF`).

## Notes and limitations

- Fixed I2C address `0x40`, fixed ~100 kHz software timing, no clock stretching.
- Readings are not CRC checked and the chip is left in its default
  (continuous) mode.
- Temperature: `raw * 165 / 65535 - 40`, humidity: `raw * 100 / 65535`.
- The chip does not answer right after power-up, so the first reading is taken
  about 8 seconds after boot.
- Each reading blocks the main loop for roughly 40 ms (20 ms pause required
  between writing the register pointer and reading it, twice per update).
- The sensor sits inside the device housing and reads a few degrees high
  because of self-heating; use an `offset` filter to calibrate.
- Tested on ESPHome 2026.10.0 with a CBU / BK7231N board. Not tested on other
  BK72xx variants.

## License

MIT
