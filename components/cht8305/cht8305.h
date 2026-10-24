#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace cht8305 {

// CHT8305 temperature/humidity sensor on a bit-banged I2C bus (bk72xx has no hardware I2C in ESPHome).
class CHT8305Component : public PollingComponent {
 public:
  void set_pins(int sda, int scl) {
    this->sda_ = sda;
    this->scl_ = scl;
  }
  void set_temperature_sensor(sensor::Sensor *s) { this->temperature_ = s; }
  void set_humidity_sensor(sensor::Sensor *s) { this->humidity_ = s; }
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

 protected:
  bool read_reg_(uint8_t reg, uint16_t *out);
  int sda_{-1};
  int scl_{-1};
  sensor::Sensor *temperature_{nullptr};
  sensor::Sensor *humidity_{nullptr};
};

}  // namespace cht8305
}  // namespace esphome
