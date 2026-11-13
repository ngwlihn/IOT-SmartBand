#include "heart_sensor.h"

#include <math.h>

#include "config.h"

namespace {
constexpr uint8_t ADDRESS = 0x57;
constexpr uint8_t REG_FIFO_DATA = 0x05;
constexpr uint8_t REG_MODE_CONFIG = 0x06;
constexpr uint8_t REG_SPO2_CONFIG = 0x07;
constexpr uint8_t REG_LED_CONFIG = 0x09;
constexpr uint16_t FINGER_IR_THRESHOLD = 1200;
constexpr uint16_t I2C_DELAY_US = 4;
}

void HeartSensor::sda(bool high) {
  if (high) pinMode(Config::MAX30100_SDA, INPUT_PULLUP);
  else { pinMode(Config::MAX30100_SDA, OUTPUT); digitalWrite(Config::MAX30100_SDA, LOW); }
  delayMicroseconds(I2C_DELAY_US);
}

void HeartSensor::scl(bool high) {
  if (high) pinMode(Config::MAX30100_SCL, INPUT_PULLUP);
  else { pinMode(Config::MAX30100_SCL, OUTPUT); digitalWrite(Config::MAX30100_SCL, LOW); }
  delayMicroseconds(I2C_DELAY_US);
}

void HeartSensor::i2cStart() { sda(true); scl(true); sda(false); scl(false); }
void HeartSensor::i2cStop() { sda(false); scl(true); sda(true); }

bool HeartSensor::i2cWrite(uint8_t value) {
  for (int bit = 7; bit >= 0; --bit) { sda(value & (1U << bit)); scl(true); scl(false); }
  sda(true); scl(true); const bool ack = digitalRead(Config::MAX30100_SDA) == LOW; scl(false); return ack;
}

uint8_t HeartSensor::i2cRead(bool ack) {
  uint8_t value = 0; sda(true);
  for (int bit = 7; bit >= 0; --bit) { scl(true); if (digitalRead(Config::MAX30100_SDA)) value |= 1U << bit; scl(false); }
  sda(!ack); scl(true); scl(false); sda(true); return value;
}

bool HeartSensor::writeRegister(uint8_t reg, uint8_t value) {
  i2cStart();
  const bool ok = i2cWrite(ADDRESS << 1) && i2cWrite(reg) && i2cWrite(value);
  i2cStop(); return ok;
}

bool HeartSensor::readRegisters(uint8_t reg, uint8_t* bytes, size_t length) {
  i2cStart();
  if (!i2cWrite(ADDRESS << 1) || !i2cWrite(reg)) { i2cStop(); return false; }
  i2cStart();
  if (!i2cWrite((ADDRESS << 1) | 1)) { i2cStop(); return false; }
  for (size_t i = 0; i < length; ++i) bytes[i] = i2cRead(i + 1 < length);
  i2cStop(); return true;
}

bool HeartSensor::begin() {
  sda(true); scl(true); delay(5);
  data_.sensorAvailable = writeRegister(REG_MODE_CONFIG, 0x40);
  if (!data_.sensorAvailable) return false;
  delay(100);
  data_.sensorAvailable = writeRegister(REG_SPO2_CONFIG, 0x47) &&
                          writeRegister(REG_LED_CONFIG, 0x77) &&
                          writeRegister(REG_MODE_CONFIG, 0x03);
  return data_.sensorAvailable;
}

bool HeartSensor::readSample(uint16_t& ir, uint16_t& red) {
  uint8_t raw[4];
  if (!readRegisters(REG_FIFO_DATA, raw, sizeof(raw))) return false;
  ir = (static_cast<uint16_t>(raw[0]) << 8) | raw[1];
  red = (static_cast<uint16_t>(raw[2]) << 8) | raw[3];
  return true;
}

void HeartSensor::processSample(uint32_t nowMs, uint16_t ir, uint16_t red) {
  data_.fingerDetected = ir >= FINGER_IR_THRESHOLD;
  if (!data_.fingerDetected) {
    data_.retainingLastResult = data_.valid && nowMs - lastValidResultMs_ <= Config::HEART_RESULT_HOLD_MS;
    if (!data_.retainingLastResult) {
      data_.valid = false;
      data_.heartRateBpm = 0;
      data_.spo2Percent = 0;
    }
    irDc_ = redDc_ = irAcEnvelope_ = redAcEnvelope_ = previousAc_ = 0;
    return;
  }
  data_.retainingLastResult = false;
  if (irDc_ == 0) { irDc_ = ir; redDc_ = red; }
  irDc_ += 0.02f * (ir - irDc_);
  redDc_ += 0.02f * (red - redDc_);
  const float irAc = ir - irDc_;
  const float redAc = red - redDc_;
  irAcEnvelope_ += 0.05f * (fabsf(irAc) - irAcEnvelope_);
  redAcEnvelope_ += 0.05f * (fabsf(redAc) - redAcEnvelope_);

  const float threshold = fmaxf(35.0f, irAcEnvelope_ * 0.55f);
  if (previousAc_ <= threshold && irAc > threshold && nowMs - lastBeatMs_ > 300) {
    if (lastBeatMs_ != 0) {
      const float bpm = 60000.0f / (nowMs - lastBeatMs_);
      if (bpm >= 35 && bpm <= 220) data_.heartRateBpm = data_.heartRateBpm == 0 ? bpm : data_.heartRateBpm * 0.75f + bpm * 0.25f;
    }
    lastBeatMs_ = nowMs;
  }
  previousAc_ = irAc;
  if (irAcEnvelope_ > 1 && redDc_ > 1 && irDc_ > 1) {
    const float ratio = (redAcEnvelope_ / redDc_) / (irAcEnvelope_ / irDc_);
    data_.spo2Percent = constrain(110.0f - 25.0f * ratio, 70.0f, 100.0f);
  }
  data_.valid = data_.heartRateBpm > 0 && nowMs - lastBeatMs_ < 3000 && data_.spo2Percent >= 70;
  if (data_.valid) lastValidResultMs_ = nowMs;
  data_.updatedAtMs = nowMs;
}

void HeartSensor::update(uint32_t nowMs) {
  if (!data_.sensorAvailable || nowMs - lastSampleMs_ < Config::HEART_SENSOR_INTERVAL_MS) return;
  lastSampleMs_ = nowMs;
  uint16_t ir, red;
  if (readSample(ir, red)) processSample(nowMs, ir, red);
  else data_.sensorAvailable = false;
}
