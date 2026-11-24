#include "gps_manager.h"

#include "config.h"

#include <math.h>

namespace {
double distanceMeters(double lat1, double lon1, double lat2, double lon2) {
  constexpr double earthRadiusM = 6371000.0;
  const double p1 = lat1 * DEG_TO_RAD;
  const double p2 = lat2 * DEG_TO_RAD;
  const double dp = (lat2 - lat1) * DEG_TO_RAD;
  const double dl = (lon2 - lon1) * DEG_TO_RAD;
  const double a = sin(dp / 2) * sin(dp / 2) + cos(p1) * cos(p2) * sin(dl / 2) * sin(dl / 2);
  return earthRadiusM * 2 * atan2(sqrt(a), sqrt(1 - a));
}
}

HardwareSerial GPS_Serial(1);

void GpsManager::begin() {
  GPS_Serial.begin(Config::GPS_BAUD, SERIAL_8N1, Config::GPS_RX, Config::GPS_TX);
}

void GpsManager::update() {
  while (GPS_Serial.available()) {
    const char c = GPS_Serial.read();
    gps_.encode(c);
  }
  const uint32_t now = millis();
  data_.ageMs = gps_.location.isValid() ? gps_.location.age() : UINT32_MAX;
  if (gps_.speed.isValid()) data_.speedKmh = gps_.speed.kmph();
  if (gps_.altitude.isValid()) data_.altitudeM = gps_.altitude.meters();
  if (gps_.satellites.isValid()) data_.satellites = gps_.satellites.value();
  if (gps_.hdop.isValid()) data_.hdop = gps_.hdop.hdop();

  if (gps_.location.isUpdated()) {
    const bool qualityOk = gps_.location.isValid() && data_.satellites >= 3 &&
                           (data_.hdop == 0 || data_.hdop <= Config::GPS_MAX_HDOP);
    if (qualityOk) acceptLocation(gps_.location.lat(), gps_.location.lng(), data_.speedKmh, now);
  }
  data_.locationValid = hasAcceptedLocation_ && now - lastAcceptedMs_ <= Config::GPS_HOLD_VALID_MS;
  data_.charsProcessed = gps_.charsProcessed();
  data_.sentencesWithFix = gps_.sentencesWithFix();
  data_.failedChecksum = gps_.failedChecksum();
}

void GpsManager::acceptLocation(double latitude, double longitude, float speedKmh, uint32_t nowMs) {
  if (!hasAcceptedLocation_) {
    data_.latitude = latitude;
    data_.longitude = longitude;
    hasAcceptedLocation_ = true;
    lastAcceptedMs_ = nowMs;
    return;
  }

  const double movement = distanceMeters(data_.latitude, data_.longitude, latitude, longitude);
  if (movement > Config::GPS_JUMP_CONFIRM_M && speedKmh < 10.0f) {
    const double pendingDistance = pendingJumpCount_ ? distanceMeters(pendingLatitude_, pendingLongitude_, latitude, longitude) : 0;
    if (!pendingJumpCount_ || pendingDistance > 25.0) {
      pendingLatitude_ = latitude;
      pendingLongitude_ = longitude;
      pendingJumpCount_ = 1;
      return;
    }
    pendingLatitude_ = pendingLatitude_ * 0.5 + latitude * 0.5;
    pendingLongitude_ = pendingLongitude_ * 0.5 + longitude * 0.5;
    if (++pendingJumpCount_ < 3) return;
    latitude = pendingLatitude_;
    longitude = pendingLongitude_;
  }
  pendingJumpCount_ = 0;

  if (movement >= Config::GPS_JITTER_DEADBAND_M) {
    const double alpha = speedKmh >= 5.0f ? 0.45 : 0.22;
    data_.latitude += (latitude - data_.latitude) * alpha;
    data_.longitude += (longitude - data_.longitude) * alpha;
  }
  lastAcceptedMs_ = nowMs;
}

GpsData GpsManager::data() const {
  return data_;
}
