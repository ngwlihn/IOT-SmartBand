#pragma once

#include <Arduino.h>

#include "system_data.h"

class HeartSensor {
 public:
  bool begin();
  void update(uint32_t nowMs);
  HeartData data() const { return data_; }

 private:
  bool writeRegister(uint8_t reg, uint8_t value);
  bool readRegisters(uint8_t reg, uint8_t* bytes, size_t length);
  bool readSample(uint16_t& ir, uint16_t& red);
  void processSample(uint32_t nowMs, uint16_t ir, uint16_t red);
  void i2cStart();
  void i2cStop();
  bool i2cWrite(uint8_t value);
  uint8_t i2cRead(bool ack);
  void sda(bool high);
  void scl(bool high);

  HeartData data_;
  float irDc_ = 0.0f;
  float redDc_ = 0.0f;
  float irAcEnvelope_ = 0.0f;
  float redAcEnvelope_ = 0.0f;
  float previousAc_ = 0.0f;
  uint32_t lastBeatMs_ = 0;
  uint32_t lastValidResultMs_ = 0;
  uint32_t lastSampleMs_ = 0;
};
