#pragma once

#include <Arduino.h>
#include <TinyGPS++.h>

#include "system_data.h"

class GpsManager {
 public:
  void begin();
  void update();
 GpsData data() const;

 private:
  void acceptLocation(double latitude, double longitude, float speedKmh, uint32_t nowMs);
  TinyGPSPlus gps_;
  GpsData data_;
  bool hasAcceptedLocation_ = false;
  uint32_t lastAcceptedMs_ = 0;
  double pendingLatitude_ = 0.0;
  double pendingLongitude_ = 0.0;
  uint8_t pendingJumpCount_ = 0;
};
