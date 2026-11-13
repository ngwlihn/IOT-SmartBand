#include "crash_detector.h"

#include <Arduino.h>

#include "config.h"
#include "signal_processing.h"

void CrashDetector::reset() {
  analyzing_ = false;
  eventStartMs_ = 0;
  startPitchDeg_ = 0.0f;
  startRollDeg_ = 0.0f;
  peakAccelerationG_ = 0.0f;
  minAccelerationG_ = 1.0f;
  peakGyroDps_ = 0.0f;
  orientationDeltaDeg_ = 0.0f;
  sampleCount_ = 0;
  inactiveCount_ = 0;
  bestAiCrashProbability_ = 0.0f;
  bestAiImpactProbability_ = 0.0f;
}

bool CrashDetector::shouldStartAnalysis(const MotionSample& sample, const AiPrediction& prediction) const {
  if (sample.linearAccelerationG >= Config::POSSIBLE_IMPACT_LINEAR_G) {
    return true;
  }
  if (sample.gyroMagnitudeDps >= Config::POSSIBLE_IMPACT_GYRO_DPS) {
    return true;
  }
  return prediction.crash >= Config::AI_SUSPECT_PROBABILITY && sample.linearAccelerationG >= 1.20f;
}

void CrashDetector::beginAnalysis(const MotionSample& sample, const AiPrediction& prediction) {
  reset();
  analyzing_ = true;
  eventStartMs_ = sample.timestampMs;
  startPitchDeg_ = sample.pitchDeg;
  startRollDeg_ = sample.rollDeg;
  updateAnalysis(sample, prediction);
}

void CrashDetector::updateAnalysis(const MotionSample& sample, const AiPrediction& prediction) {
  if (!analyzing_) {
    return;
  }

  ++sampleCount_;
  peakAccelerationG_ = max(peakAccelerationG_, sample.linearAccelerationG);
  minAccelerationG_ = min(minAccelerationG_, sample.linearAccelerationG);
  peakGyroDps_ = max(peakGyroDps_, sample.gyroMagnitudeDps);
  orientationDeltaDeg_ = max(orientationDeltaDeg_, SignalProcessing::orientationDeltaDeg(
                                                    startPitchDeg_, startRollDeg_, sample.pitchDeg, sample.rollDeg));
  bestAiCrashProbability_ = max(bestAiCrashProbability_, prediction.crash);
  bestAiImpactProbability_ = max(bestAiImpactProbability_, prediction.impact);

  const bool postImpactStable = sample.timestampMs > eventStartMs_ + 500 &&
                                sample.linearAccelerationG <= Config::INACTIVITY_LINEAR_G &&
                                sample.gyroMagnitudeDps <= Config::INACTIVITY_GYRO_DPS;
  if (postImpactStable) {
    ++inactiveCount_;
  }
}

bool CrashDetector::analysisReady(uint32_t nowMs) const {
  return analyzing_ && (nowMs - eventStartMs_) >= Config::POST_IMPACT_ANALYSIS_MS;
}

CrashEvidence CrashDetector::evidence() const {
  CrashEvidence result;
  const float inactivityRatio = sampleCount_ == 0 ? 0.0f : static_cast<float>(inactiveCount_) / static_cast<float>(sampleCount_);
  const float aiEvidence = max(bestAiCrashProbability_, bestAiImpactProbability_ * 0.55f);
  const float impactEvidence = SignalProcessing::mapToScore(peakAccelerationG_, Config::POSSIBLE_IMPACT_LINEAR_G, Config::IMPACT_THRESHOLD_G);
  const float rotationEvidence = SignalProcessing::mapToScore(peakGyroDps_, Config::POSSIBLE_IMPACT_GYRO_DPS, Config::GYRO_THRESHOLD_DPS);
  const float orientationEvidence = SignalProcessing::mapToScore(orientationDeltaDeg_, Config::ORIENTATION_THRESHOLD_DEG * 0.4f, Config::ORIENTATION_THRESHOLD_DEG);
  const float inactivityEvidence = SignalProcessing::clamp01(inactivityRatio * 2.0f);

  result.aiScore = weightedScore(aiEvidence, Config::WEIGHT_AI);
  result.impactScore = weightedScore(impactEvidence, Config::WEIGHT_IMPACT);
  result.rotationScore = weightedScore(rotationEvidence, Config::WEIGHT_ROTATION);
  result.orientationScore = weightedScore(orientationEvidence, Config::WEIGHT_ORIENTATION);
  result.inactivityScore = weightedScore(inactivityEvidence, Config::WEIGHT_INACTIVITY);
  result.totalScore = result.aiScore + result.impactScore + result.rotationScore + result.orientationScore + result.inactivityScore;
  result.peakAccelerationG = peakAccelerationG_;
  result.peakGyroDps = peakGyroDps_;
  result.orientationDeltaDeg = orientationDeltaDeg_;
  result.inactivityRatio = inactivityRatio;
  return result;
}

bool CrashDetector::crashConfirmed() const {
  const CrashEvidence currentEvidence = evidence();

  // GUARD 1: Bắt buộc phải có va đập tối thiểu 1.95g!
  // Xoay cổ tay thuần túy hoặc cử động tay thông thường có peakAcceleration < 1.2g.
  // Không một cú ngã nào xuống sàn mà không có va đập ít nhất 1.95g.
  if (peakAccelerationG_ < 1.95f) {
    return false;
  }

  // GUARD 2 (Triệt tiêu vận động thường ngày & Đi bộ / Vung tay liên tục):
  if (currentEvidence.inactivityRatio < 0.20f) {
    return false;
  }

  // GUARD 3 (Kiểm tra lật đổi tư thế cơ thể bắt buộc):
  // Vỗ tay hoặc cử động tại chỗ: góc thân người không đổi (orientationDelta < 35 độ).
  // Khi ngã thật, người đổ xuống sàn thì góc nghiêng luôn thay đổi rõ rệt (>= 35 độ).
  // (Ngoại trừ trường hợp va đập cực mạnh > 4.0g như bị tông xe).
  if (orientationDeltaDeg_ < 35.0f && peakAccelerationG_ < 4.0f) {
    return false;
  }

  // GUARD 4 (Chống báo động giả khi tháo vòng tay để lên bàn - Inanimate Table-Drop Guard):
  // Khi tháo vòng tay đặt hoặc thả nhẹ lên bàn ở cự ly gần:
  // - Không có pha rơi tự do từ độ cao (minAccelerationG > 0.55g)
  // - Vận tốc góc xoay của cơ thể ngã rất thấp (peakGyroDps < 320 dps)
  // - Va chạm vừa phải (peakAcceleration < 3.5g)
  // - Sau đó nằm yên bất động 100% trên mặt bàn (inactivityRatio >= 0.85)
  // -> Đây là thao tác tháo đồng hồ để lên bàn, KHÔNG PHẢI người thật ngã!
  if (minAccelerationG_ > 0.55f && peakGyroDps_ < 320.0f && peakAccelerationG_ < 3.50f && currentEvidence.inactivityRatio >= 0.85f) {
    return false;
  }

  // 2 NHÁNH XÁC NHẬN TAI NẠN:
  // Nhánh 1: Vấp ngã va đập thông thường (Dynamic Fall with Impact)
  // Bắt buộc: Va chạm mạnh (>= 2.20g) VÀ Góc nghiêng lật đổi (>= 38 độ) VÀ Điểm tổng hợp >= 74
  const bool dynamicFall = (peakAccelerationG_ >= 2.20f) &&
                           (orientationDeltaDeg_ >= Config::ORIENTATION_THRESHOLD_DEG) &&
                           (currentEvidence.totalScore >= Config::CRASH_SCORE_THRESHOLD);

  // Nhánh 2: Đột quỵ / Tụt huyết áp / Ngất sụp xuống (Syncope Collapse)
  // Có va chạm tiếp đất (>= 1.95g), góc nghiêng lật đổ (>= 35 độ), nằm bất động kéo dài (>= 50%) và điểm cao
  const bool syncopeFall = (peakAccelerationG_ >= 1.95f) &&
                           (orientationDeltaDeg_ >= 35.0f) &&
                           (currentEvidence.inactivityRatio >= 0.50f) &&
                           (bestAiCrashProbability_ >= Config::AI_CRASH_PROBABILITY && currentEvidence.totalScore >= 70);

  return dynamicFall || syncopeFall;
}

uint8_t CrashDetector::weightedScore(float value01, uint8_t weight) const {
  const float bounded = SignalProcessing::clamp01(value01);
  return static_cast<uint8_t>(roundf(bounded * static_cast<float>(weight)));
}
