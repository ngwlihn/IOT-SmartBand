#include "display_manager.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <qrcode.h>

#include "config.h"
#include "text_utils.h"

namespace {
Adafruit_SSD1306 display(Config::OLED_WIDTH, Config::OLED_HEIGHT, &Wire1, -1);
}

bool DisplayManager::begin() {
  Wire1.begin(Config::OLED_SDA, Config::OLED_SCL);
  Wire1.setClock(400000);
  available_ = display.begin(SSD1306_SWITCHCAPVCC, Config::OLED_ADDRESS);
  reset();
  if (available_) display.display();
  return available_;
}

void DisplayManager::showWelcome() {
  reset();
  center("WELCOME", 2, 1);
  center("AI CRASH", 18, 2);
  center("DETECTION", 42, 2);
  if (available_) display.display();
}

void DisplayManager::showSystemReady() {
  reset();
  center("SYSTEM READY", 8, 1);
  center("NORMAL", 28, 2);
  if (available_) display.display();
}

void DisplayManager::showTimeWeather(const tm& timeInfo, bool timeValid, const WeatherData& weather) {
  reset();
  char line[24];
  if (timeValid) {
    strftime(line, sizeof(line), "%H:%M:%S", &timeInfo);
    center(line, 4, 2);
    strftime(line, sizeof(line), "%d/%m/%Y", &timeInfo);
    center(line, 26, 1);
  } else {
    center("SMARTBAND", 10, 2);
    center("SEARCHING...", 34, 1);
  }
  if (weather.valid) {
    snprintf(line, sizeof(line), "%.1fC  %s", weather.temperatureC, weatherName(weather.weatherCode));
    center(line, 46, 1);
  } else {
    center("STATUS: SAFE", 46, 1);
  }
  if (available_) display.display();
}

void DisplayManager::showAiLocation(const AiPrediction& prediction, const GpsData& gps) {
  reset();
  char line[24];
  snprintf(line, sizeof(line), "AI:%s", motionClassName(prediction.predictedClass));
  for (char* p = line; *p; ++p) *p = toupper(*p);
  center(line, 0, 2);
  center(gps.locationValid ? "GPS: FIX" : "GPS: SEARCH", 22, 1);
  if (gps.locationValid) {
    snprintf(line, sizeof(line), "LAT: %.5f", gps.latitude);
    center(line, 38, 1);
    snprintf(line, sizeof(line), "LON: %.5f", gps.longitude);
    center(line, 52, 1);
  } else {
    center("NO LOCATION", 46, 1);
  }
  if (available_) display.display();
}

void DisplayManager::showGpsStatus(const GpsData& gps) {
  reset();
  center("GPS STATUS", 0, 1);
  char line[22];
  if (gps.charsProcessed == 0) {
    center("NO NMEA DATA", 19, 1);
    center("GPS TX -> GPIO2", 37, 1);
    center("BAUD: 9600", 52, 1);
  } else {
    snprintf(line, sizeof(line), "NMEA:%lu", gps.charsProcessed);
    center(line, 14, 1);
    snprintf(line, sizeof(line), "SAT:%lu HDOP:%.1f", gps.satellites, gps.hdop);
    center(line, 27, 1);
    snprintf(line, sizeof(line), "SPEED:%.1f KM/H", gps.speedKmh);
    center(line, 40, 1);
    snprintf(line, sizeof(line), "ALT:%.0fM %s", gps.altitudeM, gps.locationValid ? "FIX" : "SEARCH");
    center(line, 53, 1);
  }
  if (available_) display.display();
}

void DisplayManager::showHeartHealth(const HeartData& heart) {
  reset();
  center("SUC KHOE", 0, 1);
  char line[24];
  if (!heart.sensorAvailable) {
    center("MAX30100", 22, 1);
    center("DANG KET NOI...", 42, 1);
  } else if (heart.valid) {
    snprintf(line, sizeof(line), "TIM: %.0f BPM", heart.heartRateBpm);
    center(line, 18, 1);
    snprintf(line, sizeof(line), "SpO2: %.0f%%", heart.spo2Percent);
    center(line, 38, 2);
    if (heart.retainingLastResult) center("KQ GAN NHAT", 55, 1);
  } else {
    center("TIM: -- BPM", 20, 1);
    center("SpO2: -- %", 38, 2);
  }
  if (available_) display.display();
}

void DisplayManager::showHealthStatus(const HeartData& heart, const AiPrediction& prediction, const GpsData& gps) {
  reset();
  char line[26];
  if (heart.valid) {
    snprintf(line, sizeof(line), "HR    : %.0f BPM", heart.heartRateBpm);
    center(line, 6, 1);
    snprintf(line, sizeof(line), "SpO2  : %.0f%%", heart.spo2Percent);
    center(line, 20, 1);
  } else {
    center("HR    : -- BPM", 6, 1);
    center("SpO2  : -- %", 20, 1);
  }

  const char* aiText = "SAFE";
  if (prediction.predictedClass == MotionClass::Impact) aiText = "IMPACT";
  else if (prediction.predictedClass == MotionClass::Crash) aiText = "FALL";
  snprintf(line, sizeof(line), "AI    : %s", aiText);
  center(line, 35, 1);

  if (gps.locationValid) {
    snprintf(line, sizeof(line), "GPS   : FIX (%lu SAT)", gps.satellites);
    center(line, 49, 1);
  } else {
    center("GPS   : SEARCHING...", 49, 1);
  }
  if (available_) display.display();
}

void DisplayManager::showCustom(const char* line1, const char* line2, const char* line3) {
  reset();
  center(line1, 2, 1);
  center(line2, 24, 2);
  center(line3, 52, 1);
  if (available_) display.display();
}

void DisplayManager::showCrashDetected(uint8_t secondsRemaining, bool movementResumed) {
  reset();
  char line[12];
  snprintf(line, sizeof(line), "%u", secondsRemaining);
  if (movementResumed) {
    center("FALL DETECTED", 2, 1);
    center("MOVING DETECTED", 16, 1);
    center(line, 28, 2);
    center("PRESS TO CANCEL", 48, 1);
  } else {
    center("FALL DETECTED", 2, 1);
    center(line, 16, 3);
    center("PRESS TO CANCEL", 48, 1);
  }
  if (available_) display.display();
}

void DisplayManager::showRecovery() {
  reset();
  center("I'M OK", 14, 2);
  center("ALERT CANCELLED", 42, 1);
  if (available_) display.display();
}

void DisplayManager::showEmergency(bool movementResumed) {
  reset();
  if (movementResumed) {
    center("EMERGENCY", 4, 2);
    center("MOVING DETECTED", 26, 1);
    center("CALLING 115...", 46, 1);
  } else {
    center("EMERGENCY", 4, 2);
    center("FALL IMMOBILE", 26, 1);
    center("CALLING 115...", 46, 1);
  }
  if (available_) display.display();
}

void DisplayManager::showEmergencyMedicalQr(const char* qrText, const HeartData& heart, const char* phone) {
  reset();
  if (!available_) return;

  // Lọc sạch dấu tiếng Việt và ký tự đặc biệt sang chuẩn ASCII an toàn cho màn hình OLED SSD1306
  String cleanPhone = TextUtils::removeDiacritics(phone ? phone : "");
  cleanPhone.trim();
  if (cleanPhone.isEmpty()) cleanPhone = "115";

  // QR points to the medical history supplied by the app/backend.
  String payload;
  if (qrText && qrText[0]) {
    payload = TextUtils::removeDiacritics(qrText);
  } else {
    payload = "LICH SU BENH: CHUA DONG BO\nLH CON CAI: " + cleanPhone;
  }

  // Tự động chọn phiên bản QR Code phù hợp với độ dài văn bản
  const size_t len = payload.length();
  uint8_t targetVer = 3;
  if (len <= 53) targetVer = 3;
  else if (len <= 78) targetVer = 4;
  else if (len <= 106) targetVer = 5;
  else if (len <= 134) targetVer = 6;
  else targetVer = 7;

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(7)];
  int8_t initRes = -1;

  for (uint8_t v = targetVer; v <= 7; ++v) {
    initRes = qrcode_initText(&qrcode, qrcodeData, v, ECC_LOW, payload.c_str());
    if (initRes == 0) break;
  }

  if (initRes != 0) {
    // Dự phòng an toàn nếu chuỗi quá dài: rút gọn để vừa Version 3
    char fallbackText[64];
    snprintf(fallbackText, sizeof(fallbackText), "LICH SU BENH\nLH CON CAI: %s", cleanPhone.c_str());
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, fallbackText);
  }

  // Nếu QR size <= 29 (Version <= 3) thì vẽ scale 2x (58x58px) siêu nét, dễ quét.
  // Nếu QR size > 29 (Version >= 4) thì vẽ scale 1x căn giữa dọc.
  const uint8_t scale = (qrcode.size <= 29) ? 2 : 1;
  const int16_t qrPx = qrcode.size * scale;
  const int16_t border = (scale == 2) ? 1 : 2;
  const int16_t startX = (scale == 2) ? 2 : 4;
  const int16_t startY = (64 - qrPx) / 2;

  // Hộp nền trắng (Quiet Zone) giúp camera mọi điện thoại nhận diện mã ngay lập tức
  display.fillRect(startX - border, startY - border, qrPx + (border * 2), qrPx + (border * 2), SSD1306_WHITE);

  // Vẽ các module QR đen trên nền trắng
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(startX + (x * scale), startY + (y * scale), scale, scale, SSD1306_BLACK);
      }
    }
  }

  // Cột thông tin hiển thị bên phải màn hình LED OLED (thuần ASCII, không lỗi font)
  const int16_t textX = startX + qrPx + border + 4;
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(textX, 6);
  if (heart.valid) display.printf("HR: %.0f", heart.heartRateBpm);
  else display.print("HR: --");

  display.setCursor(textX, 20);
  if (heart.valid) display.printf("SpO2:%.0f%%", heart.spo2Percent);
  else display.print("SpO2:--");

  display.setCursor(textX, 36);
  display.print("CALL:");

  display.setCursor(textX, 48);
  display.printf("%.11s", cleanPhone.c_str());

  display.display();
}

void DisplayManager::showSos() {
  reset();
  center("SOS", 8, 3);
  center("CALLING 115...", 44, 1);
  if (available_) display.display();
}

void DisplayManager::showCalibration() {
  reset();
  center("CALIBRATING", 10, 1);
  center("KEEP STILL", 32, 2);
  if (available_) display.display();
}

void DisplayManager::showCalibrationOk() {
  reset();
  center("CALIBRATION", 12, 1);
  center("OK", 30, 2);
  if (available_) display.display();
}

void DisplayManager::showError(const char* title, const char* detail) {
  reset();
  center(title, 12, 1);
  center(detail, 34, 1);
  if (available_) display.display();
}

void DisplayManager::reset() {
  if (!available_) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
}

void DisplayManager::center(const char* text, int16_t y, uint8_t size) {
  if (!available_ || !text) return;
  String clean = TextUtils::removeDiacritics(text);
  int16_t x1, y1;
  uint16_t width, height;
  display.setTextSize(size);
  display.getTextBounds(clean.c_str(), 0, y, &x1, &y1, &width, &height);
  display.setCursor(max(0, (128 - static_cast<int>(width)) / 2), y);
  display.print(clean.c_str());
}

const char* DisplayManager::weatherName(int code) const {
  if (code == 0) return "CLEAR";
  if (code <= 3) return "CLOUDY";
  if (code <= 48) return "FOG";
  if (code <= 67) return "RAIN";
  if (code <= 77) return "SNOW";
  if (code <= 82) return "SHOWER";
  return "STORM";
}
