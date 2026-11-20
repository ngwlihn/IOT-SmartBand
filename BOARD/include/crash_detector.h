#pragma once

#include "motion_types.h"
#include "system_data.h"

struct CrashEvidence {
  uint8_t aiScore = 0;
  uint8_t impactScore = 0;
  uint8_t rotationScore = 0;
  uint8_t orientationScore = 0;
  uint8_t inactivityScore = 0;
  uint8_t heartScore = 0;
  uint8_t totalScore = 0;
  float peakAccelerationG = 0.0f;
  float peakGyroDps = 0.0f;
  float orientationDeltaDeg = 0.0f;
  float inactivityRatio = 0.0f;
  float heartRateBpm = 0.0f;
  float spo2Percent = 0.0f;
  bool cardiacCrisis = false;
  bool fingerPresent = true;
  bool movementResumed = false;
  const char* fallType = "NORMAL"; // "FALL_IMPACT", "FALL_CARDIAC", "FALL_RECOVERED"
};

class CrashDetector {
 public:
  void reset();
  bool shouldStartAnalysis(const MotionSample& sample, const AiPrediction& prediction, const HeartData& heart) const;
  void beginAnalysis(const MotionSample& sample, const AiPrediction& prediction, const HeartData& heart);
  void updateAnalysis(const MotionSample& sample, const AiPrediction& prediction, const HeartData& heart);
  void updateConfirmation(const MotionSample& sample, const HeartData& heart);
  bool analysisReady(uint32_t nowMs) const;
  CrashEvidence evidence() const;
  bool crashConfirmed() const;
  const char* fallType() const { return currentFallType_; }
  bool movementResumed() const { return movementResumed_; }

 private:
  uint8_t weightedScore(float value01, uint8_t weight) const;

  bool analyzing_ = false;
  uint32_t eventStartMs_ = 0;
  float startPitchDeg_ = 0.0f;
  float startRollDeg_ = 0.0f;
  float peakAccelerationG_ = 0.0f;
  float minAccelerationG_ = 1.0f;
  float peakGyroDps_ = 0.0f;
  float orientationDeltaDeg_ = 0.0f;
  uint32_t sampleCount_ = 0;
  uint32_t inactiveCount_ = 0;
  float bestAiCrashProbability_ = 0.0f;
  float bestAiImpactProbability_ = 0.0f;

  // Hợp nhất sinh lý tim mạch (Physiological Fusion)
  float heartRateAtFall_ = 0.0f;
  float spo2AtFall_ = 0.0f;
  bool cardiacCrisis_ = false;
  bool fingerDetected_ = true;

  // Xác nhận cử động giai đoạn 2 (Two-phase verification: không tự hủy, nhưng gắn nhãn cảnh báo)
  bool movementResumed_ = false;
  uint32_t confirmationSamples_ = 0;
  uint32_t confirmationActiveSamples_ = 0;
  const char* currentFallType_ = "NORMAL";
};
