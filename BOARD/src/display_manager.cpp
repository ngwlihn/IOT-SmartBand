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
  center("VONG TAY", 8, 2);
  center("Y TE", 28, 2);
  center("KHOI DONG...", 50, 1);
  if (available_) display.display();
}

void DisplayManager::showSystemReady() {
  reset();
  center("HE THONG", 8, 1);
  center("SAN SANG", 24, 2);
  center("AN TOAN", 48, 1);
  if (available_) display.display();
}

void DisplayManager::showTimeWeather(const tm& timeInfo, bool timeValid, const WeatherData& weather) {
  reset();
  char line[24];
  tm localTime = timeInfo;
  if (!timeValid) {
    uint32_t sec = millis() / 1000;
    localTime.tm_hour = (16 + (sec / 3600)) % 24;
    localTime.tm_min = (15 + (sec / 60)) % 60;
    localTime.tm_sec = sec % 60;
    localTime.tm_mday = 24;
    localTime.tm_mon = 10;
    localTime.tm_year = 126; // 2026
  }

  strftime(line, sizeof(line), "%H:%M:%S", &localTime);
  center(line, 4, 2);
  strftime(line, sizeof(line), "%d/%m/%Y", &localTime);
  center(line, 26, 1);

  if (weather.valid) {
    snprintf(line, sizeof(line), "%.0fC %s", weather.temperatureC, weatherShortName(weather.weatherCode));
  } else {
    snprintf(line, sizeof(line), "28C NANG");
  }
  center(line, 42, 2);
  if (available_) display.display();
}

void DisplayManager::showAiLocation(const AiPrediction& prediction, const GpsData& gps) {
  reset();
  char line[24];
  const char* aiText = "AN TOAN";
  if (prediction.predictedClass == MotionClass::Impact) aiText = "VA CHAM";
  else if (prediction.predictedClass == MotionClass::Crash) aiText = "CO NGA";
  snprintf(line, sizeof(line), "AI: %s", aiText);
  center(line, 0, 2);
  center(gps.locationValid ? "DINH VI: SAN SANG" : "DINH VI: DANG TIM", 22, 1);
  if (gps.locationValid) {
    snprintf(line, sizeof(line), "VI DO : %.5f", gps.latitude);
    center(line, 38, 1);
    snprintf(line, sizeof(line), "KINH DO: %.5f", gps.longitude);
    center(line, 52, 1);
  } else {
    center("CHUA CO TOA DO", 46, 1);
  }
  if (available_) display.display();
}

void DisplayManager::showGpsStatus(const GpsData& gps) {
  reset();
  char line[26];
  if (gps.charsProcessed == 0) {
    center("GPS: CHUA KET NOI", 12, 1);
    center("KIEM TRA DAY NOI", 28, 1);
    center("TX -> GPIO6 (RX)", 44, 1);
    if (available_) display.display();
    return;
  }

  // Dòng 1 (y=2): Trạng thái Fix và số lượng vệ tinh
  if (gps.locationValid) {
    snprintf(line, sizeof(line), "GPS: SAN SANG (%lu VT)", gps.satellites);
  } else if (gps.satellites > 0) {
    snprintf(line, sizeof(line), "GPS: TIM (%lu VT)", gps.satellites);
  } else {
    snprintf(line, sizeof(line), "GPS: DANG TIM VE TINH");
  }
  center(line, 2, 1);

  // Dòng 2 (y=14): Vĩ độ (Latitude)
  if (gps.locationValid) {
    snprintf(line, sizeof(line), "VI DO  : %.5f", gps.latitude);
  } else {
    snprintf(line, sizeof(line), "VI DO  : DANG DO...");
  }
  center(line, 14, 1);

  // Dòng 3 (y=26): Kinh độ (Longitude)
  if (gps.locationValid) {
    snprintf(line, sizeof(line), "KINH DO: %.5f", gps.longitude);
  } else {
    snprintf(line, sizeof(line), "KINH DO: DANG DO...");
  }
  center(line, 26, 1);

  // Dòng 4 (y=38): Độ chính xác HDOP và Độ cao
  snprintf(line, sizeof(line), "HDOP:%.1f  CAO:%.0fM", gps.hdop, gps.altitudeM);
  center(line, 38, 1);

  // Dòng 5 (y=50): Tốc độ và Số lượng gói NMEA đã xử lý
  snprintf(line, sizeof(line), "%.1fKM/H  NMEA:%lu", gps.speedKmh, gps.charsProcessed);
  center(line, 50, 1);

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
    snprintf(line, sizeof(line), "NHIP TIM: %.0f L/P", heart.heartRateBpm);
    center(line, 18, 1);
    snprintf(line, sizeof(line), "SpO2: %.0f%%", heart.spo2Percent);
    center(line, 38, 2);
    if (heart.retainingLastResult) center("KQ GAN NHAT", 55, 1);
  } else {
    center("NHIP TIM: -- L/P", 20, 1);
    center("SpO2: -- %", 38, 2);
  }
  if (available_) display.display();
}

void DisplayManager::showHealthStatus(const HeartData& heart, const AiPrediction& prediction, const GpsData& gps, const char* bloodGroup) {
  reset();
  char line[26];

  // Dòng 1 (Size 2, y=3): Nhịp tim
  if (heart.valid) {
    snprintf(line, sizeof(line), "TIM: %.0f", heart.heartRateBpm);
  } else {
    snprintf(line, sizeof(line), "TIM: --");
  }
  center(line, 3, 2);

  // Dòng 2 (Size 2, y=23): Nồng độ oxy SpO2
  if (heart.valid) {
    snprintf(line, sizeof(line), "SpO2:%.0f%%", heart.spo2Percent);
  } else {
    snprintf(line, sizeof(line), "SpO2:--%%");
  }
  center(line, 23, 2);

  // Dòng 3 (Size 2, y=44): Trạng thái Cử động AI cỡ to
  const char* aiText = "AN TOAN";
  if (prediction.predictedClass == MotionClass::Impact) aiText = "VA CHAM";
  else if (prediction.predictedClass == MotionClass::Crash) aiText = "CO NGA";
  center(aiText, 44, 2);

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
    center("PHAT HIEN NGA", 2, 1);
    center("DA CU DONG LAI", 16, 1);
    center(line, 28, 2);
    center("BAM NUT DE HUY", 48, 1);
  } else {
    center("PHAT HIEN NGA", 2, 1);
    center(line, 16, 3);
    center("BAM NUT DE HUY", 48, 1);
  }
  if (available_) display.display();
}

void DisplayManager::showRecovery() {
  reset();
  center("TOI ON", 14, 2);
  center("DA HUY BAO DONG", 42, 1);
  if (available_) display.display();
}

void DisplayManager::showEmergency(bool movementResumed) {
  reset();
  if (movementResumed) {
    center("CAP CUU", 4, 2);
    center("DA CU DONG LAI", 26, 1);
    center("DANG GOI 115...", 46, 1);
  } else {
    center("CAP CUU", 4, 2);
    center("NAN NHAN NGA", 26, 1);
    center("DANG GOI 115...", 46, 1);
  }
  if (available_) display.display();
}

void DisplayManager::showEmergencyMedicalQr(const char* qrText, const HeartData& heart, const char* phone, const char* bloodGroup) {
  reset();
  if (!available_) return;

  // Lọc sạch dấu tiếng Việt và ký tự đặc biệt sang chuẩn ASCII an toàn cho màn hình OLED SSD1306
  String cleanBlood = TextUtils::removeDiacritics(bloodGroup ? bloodGroup : "");
  cleanBlood.trim();
  cleanBlood.toUpperCase();
  if (cleanBlood.isEmpty() || cleanBlood.indexOf("CHUA") >= 0) {
    cleanBlood = "--";
  }

  String cleanPhone = TextUtils::removeDiacritics(phone ? phone : "");
  cleanPhone.trim();
  if (cleanPhone.isEmpty()) cleanPhone = "115";

  // QR points to the medical history supplied by the app/backend.
  String payload;
  if (qrText && qrText[0]) {
    payload = TextUtils::removeDiacritics(qrText);
  } else {
    payload = "THONG TIN NAN NHAN\nNhom mau: " + cleanBlood + "\nLH CON CAI: " + cleanPhone;
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
    snprintf(fallbackText, sizeof(fallbackText), "MAU: %s\nLH CON CAI: %s", cleanBlood.c_str(), cleanPhone.c_str());
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

  display.setCursor(textX, 2);
  display.printf("MAU :%s", cleanBlood.c_str());

  display.setCursor(textX, 14);
  if (heart.valid) display.printf("TIM :%.0f", heart.heartRateBpm);
  else display.print("TIM :--");

  display.setCursor(textX, 26);
  if (heart.valid) display.printf("SpO2:%.0f%%", heart.spo2Percent);
  else display.print("SpO2:--");

  display.setCursor(textX, 39);
  display.print("GOI :");

  display.setCursor(textX, 51);
  display.printf("%.10s", cleanPhone.c_str());

  display.display();
}

void DisplayManager::showSos() {
  reset();
  center("SOS", 8, 3);
  center("DANG GOI 115...", 44, 1);
  if (available_) display.display();
}

void DisplayManager::showCalibration() {
  reset();
  center("CAN CHINH AI", 16, 1);
  center("GIU YEN TAY...", 36, 1);
  if (available_) display.display();
}

void DisplayManager::showCalibrationOk() {
  reset();
  center("CAN CHINH XONG", 16, 1);
  center("SAN SANG", 34, 2);
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
  if (code == 0) return "TROI NANG";
  if (code <= 3) return "CO MAY";
  if (code <= 48) return "SUONG MU";
  if (code <= 67) return "CO MUA";
  if (code <= 77) return "CO TUYET";
  if (code <= 82) return "MUA RAO";
  return "GIONG BAO";
}

const char* DisplayManager::weatherShortName(int code) const {
  if (code == 0) return "NANG";
  if (code <= 3) return "MAY";
  if (code <= 48) return "SUONG";
  if (code <= 67) return "MUA";
  if (code <= 77) return "TUYET";
  if (code <= 82) return "RAO";
  return "BAO";
}
