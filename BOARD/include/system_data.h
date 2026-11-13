#pragma once

#include <Arduino.h>

enum class WeatherStatus : uint8_t {
  WaitGps,
  WaitWifi,
  Fetching,
  Ready,
  Error
};

struct GpsData {
  bool locationValid = false;
  double latitude = 0.0;
  double longitude = 0.0;
  float speedKmh = 0.0f;
  float altitudeM = 0.0f;
  uint32_t satellites = 0;
  uint32_t ageMs = 0;
  uint32_t charsProcessed = 0;
  uint32_t sentencesWithFix = 0;
  uint32_t failedChecksum = 0;
  float hdop = 0.0f;
};

struct WeatherData {
  bool valid = false;
  WeatherStatus status = WeatherStatus::WaitGps;
  float temperatureC = 0.0f;
  uint8_t rainProbability = 0;
  int weatherCode = 0;
};

struct HeartData {
  bool sensorAvailable = false;
  bool fingerDetected = false;
  bool valid = false;
  bool retainingLastResult = false;
  float heartRateBpm = 0.0f;
  float spo2Percent = 0.0f;
  uint32_t updatedAtMs = 0;
};
