#include "cht8305.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include <Arduino.h>

namespace esphome {
namespace cht8305 {

static const char *const TAG = "cht8305";
static const uint8_t ADDR = 0x40;
static const uint8_t REG_TEMPERATURE = 0x00;
static const uint8_t REG_HUMIDITY = 0x01;
static const uint32_t HALF_BIT_US = 5;

static inline void release_(int pin) { pinMode(pin, INPUT_PULLUP); }
static inline void drive_low_(int pin) {
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
}

static void start_(int sda, int scl) {
  release_(sda);
  release_(scl);
  delayMicroseconds(HALF_BIT_US);
  drive_low_(sda);
  delayMicroseconds(HALF_BIT_US);
  drive_low_(scl);
}

static void stop_(int sda, int scl) {
  drive_low_(sda);
  delayMicroseconds(HALF_BIT_US);
  release_(scl);
  delayMicroseconds(HALF_BIT_US);
  release_(sda);
  delayMicroseconds(HALF_BIT_US);
}

static bool write_byte_(int sda, int scl, uint8_t b) {
  for (int i = 7; i >= 0; i--) {
    if (b & (1 << i)) release_(sda); else drive_low_(sda);
    delayMicroseconds(HALF_BIT_US);
    release_(scl);
    delayMicroseconds(HALF_BIT_US);
    drive_low_(scl);
  }
  release_(sda);
  delayMicroseconds(HALF_BIT_US);
  release_(scl);
  delayMicroseconds(HALF_BIT_US);
  bool ack = digitalRead(sda) == LOW;
  drive_low_(scl);
  return ack;
}

static uint8_t read_byte_(int sda, int scl, bool ack) {
  uint8_t v = 0;
  release_(sda);
  for (int i = 0; i < 8; i++) {
    delayMicroseconds(HALF_BIT_US);
    release_(scl);
    delayMicroseconds(HALF_BIT_US);
    v = (v << 1) | (digitalRead(sda) == HIGH ? 1 : 0);
    drive_low_(scl);
  }
  if (ack) drive_low_(sda); else release_(sda);
  delayMicroseconds(HALF_BIT_US);
  release_(scl);
  delayMicroseconds(HALF_BIT_US);
  drive_low_(scl);
  release_(sda);
  return v;
}

bool CHT8305Component::read_reg_(uint8_t reg, uint16_t *out) {
  start_(this->sda_, this->scl_);
  bool ok = write_byte_(this->sda_, this->scl_, ADDR << 1);
  ok = write_byte_(this->sda_, this->scl_, reg) && ok;
  stop_(this->sda_, this->scl_);
  if (!ok)
    return false;
  delay(20);  // the chip needs a pause between pointer write and read
  start_(this->sda_, this->scl_);
  ok = write_byte_(this->sda_, this->scl_, (ADDR << 1) | 1);
  uint8_t hi = read_byte_(this->sda_, this->scl_, true);
  uint8_t lo = read_byte_(this->sda_, this->scl_, false);
  stop_(this->sda_, this->scl_);
  *out = (hi << 8) | lo;
  return ok;
}

void CHT8305Component::setup() {
  release_(this->sda_);
  release_(this->scl_);
  // the chip is not ready right after power-up; take the first reading shortly after boot
  this->set_timeout(8000, [this]() { this->update(); });
}

void CHT8305Component::dump_config() {
  ESP_LOGCONFIG(TAG, "CHT8305:");
  ESP_LOGCONFIG(TAG, "  SDA pin: %d, SCL pin: %d (software I2C)", this->sda_, this->scl_);
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature", this->temperature_);
  LOG_SENSOR("  ", "Humidity", this->humidity_);
}

void CHT8305Component::update() {
  uint16_t t_raw, h_raw;
  if (this->temperature_ != nullptr) {
    if (this->read_reg_(REG_TEMPERATURE, &t_raw)) {
      this->temperature_->publish_state(t_raw * 165.0f / 65535.0f - 40.0f);
    } else {
      this->status_set_warning();
      return;
    }
  }
  if (this->humidity_ != nullptr) {
    if (this->read_reg_(REG_HUMIDITY, &h_raw)) {
      this->humidity_->publish_state(h_raw * 100.0f / 65535.0f);
    } else {
      this->status_set_warning();
      return;
    }
  }
  this->status_clear_warning();
}

}  // namespace cht8305
}  // namespace esphome
