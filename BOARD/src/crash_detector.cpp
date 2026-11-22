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

  heartRateAtFall_ = 0.0f;
  spo2AtFall_ = 0.0f;
  cardiacCrisis_ = false;
  normalHeartConfirmed_ = false;
  fingerDetected_ = true;

  impactSpikeCount_ = 0;
  previousSampleHigh_ = false;
  gyroReversals_ = 0;
  lastGyroSign_ = 0.0f;

  movementResumed_ = false;
  confirmationSamples_ = 0;
  confirmationActiveSamples_ = 0;
  currentFallType_ = "NORMAL";
}

bool CrashDetector::shouldStartAnalysis(const MotionSample& sample, const AiPrediction& prediction, const HeartData& heart) const {
  // Khoa bao ve: Neu cam bien nhip tim hoat dong ma khong phat hien tiep xuc co tay/ngon tay -> khong phan tich
  if (heart.sensorAvailable && !heart.fingerDetected) {
    return false;
  }

  if (sample.linearAccelerationG >= Config::POSSIBLE_IMPACT_LINEAR_G) {
    return true;
  }
  if (sample.gyroMagnitudeDps >= Config::POSSIBLE_IMPACT_GYRO_DPS) {
    return true;
  }
  if (prediction.crash >= Config::AI_SUSPECT_PROBABILITY && sample.linearAccelerationG >= 1.20f) {
    return true;
  }

  // Tình huống Đột quỵ / Tụt huyết áp / Suy tim (Cardiac Crisis Trigger):
  // Nhịp tim tăng vọt do hoảng loạn hoặc tụt dốc nguy kịch, kèm trượt ngã nhẹ (>= 1.35g)
  if (heart.valid) {
    const bool cardiacAnomaly = (heart.heartRateBpm >= Config::CARDIAC_TACHYCARDIA_BPM) ||
                                (heart.heartRateBpm <= Config::CARDIAC_BRADYCARDIA_BPM) ||
                                (heart.spo2Percent <= Config::CARDIAC_HYPOXIA_SPO2);
    if (cardiacAnomaly && sample.linearAccelerationG >= 1.35f) {
      return true;
    }
  }

  return false;
}

void CrashDetector::beginAnalysis(const MotionSample& sample, const AiPrediction& prediction, const HeartData& heart) {
  reset();
  analyzing_ = true;
  eventStartMs_ = sample.timestampMs;
  startPitchDeg_ = sample.pitchDeg;
  startRollDeg_ = sample.rollDeg;
  if (heart.sensorAvailable) {
    fingerDetected_ = heart.fingerDetected;
  }
  updateAnalysis(sample, prediction, heart);
}

void CrashDetector::updateAnalysis(const MotionSample& sample, const AiPrediction& prediction, const HeartData& heart) {
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

  // Phat hien hien tuong Lac tay (Shake detection via repetitive oscillation spikes):
  if (sample.linearAccelerationG >= Config::POSSIBLE_IMPACT_LINEAR_G) {
    if (!previousSampleHigh_) {
      ++impactSpikeCount_;
      previousSampleHigh_ = true;
    }
  } else if (sample.linearAccelerationG < 0.90f) {
    previousSampleHigh_ = false;
  }

  // Dem so lan doi chieu con quay hoi chuyen (Lac qua lai lien tuc):
  if (fabsf(sample.gx) > 160.0f) {
    const float currentSign = sample.gx > 0.0f ? 1.0f : -1.0f;
    if (lastGyroSign_ != 0.0f && currentSign != lastGyroSign_) {
      ++gyroReversals_;
    }
    lastGyroSign_ = currentSign;
  }

  const bool postImpactStable = sample.timestampMs > eventStartMs_ + 500 &&
                                sample.linearAccelerationG <= Config::INACTIVITY_LINEAR_G &&
                                sample.gyroMagnitudeDps <= Config::INACTIVITY_GYRO_DPS;
  if (postImpactStable) {
    ++inactiveCount_;
  }

  // Thu thập dữ liệu sinh lý học MAX30100 PPG
  if (heart.sensorAvailable) {
    fingerDetected_ = heart.fingerDetected;
    if (heart.valid) {
      heartRateAtFall_ = heart.heartRateBpm;
      spo2AtFall_ = heart.spo2Percent;
      if (heart.heartRateBpm >= Config::CARDIAC_TACHYCARDIA_BPM ||
          heart.heartRateBpm <= Config::CARDIAC_BRADYCARDIA_BPM ||
          heart.spo2Percent <= Config::CARDIAC_HYPOXIA_SPO2) {
        cardiacCrisis_ = true;
        normalHeartConfirmed_ = false;
      } else if (heart.heartRateBpm >= 55.0f && heart.heartRateBpm <= 100.0f) {
        normalHeartConfirmed_ = true;
      }
    }
  }
}

void CrashDetector::updateConfirmation(const MotionSample& sample, const HeartData& heart) {
  // Quan sát cử động giai đoạn 2 (trong 10s UserConfirmation)
  // LƯU Ý THEO YÊU CẦU: TUYỆT ĐỐI KHÔNG TỰ HỦY BÁO ĐỘNG!
  // Chỉ nhận diện nạn nhân đã đứng dậy cử động để gắn nhãn phân loại (FALL_RECOVERED),
  // cảnh báo gửi về điện thoại người nhà để theo dõi vết thương kín / chấn thương muộn.
  ++confirmationSamples_;
  if (sample.linearAccelerationG > 0.35f || sample.gyroMagnitudeDps > 45.0f) {
    ++confirmationActiveSamples_;
  }

  if (confirmationSamples_ >= 25) { // Sau ít nhất 0.5s quan sát
    const float activeRatio = static_cast<float>(confirmationActiveSamples_) / static_cast<float>(confirmationSamples_);
    if (activeRatio >= 0.35f) {
      movementResumed_ = true;
      currentFallType_ = "FALL_RECOVERED";
    }
  }

  if (heart.valid && heartRateAtFall_ == 0.0f) {
    heartRateAtFall_ = heart.heartRateBpm;
    spo2AtFall_ = heart.spo2Percent;
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

  // Điểm số sinh lý tim mạch (Heart Evidence: 0 - 1.0)
  float heartEvidence = 0.3f; // Mặc định bình thường
  if (cardiacCrisis_) {
    heartEvidence = 1.0f; // Bất thường tim mạch: tối đa điểm
  } else if (heartRateAtFall_ > 0.0f) {
    heartEvidence = 0.5f; // Có nhịp tim người đeo
  }

  result.aiScore = weightedScore(aiEvidence, Config::WEIGHT_AI);
  result.impactScore = weightedScore(impactEvidence, Config::WEIGHT_IMPACT);
  result.rotationScore = weightedScore(rotationEvidence, Config::WEIGHT_ROTATION);
  result.orientationScore = weightedScore(orientationEvidence, Config::WEIGHT_ORIENTATION);
  result.inactivityScore = weightedScore(inactivityEvidence, Config::WEIGHT_INACTIVITY);
  result.heartScore = weightedScore(heartEvidence, Config::WEIGHT_HEART);
  result.totalScore = result.aiScore + result.impactScore + result.rotationScore + result.orientationScore + result.inactivityScore + result.heartScore;

  result.peakAccelerationG = peakAccelerationG_;
  result.peakGyroDps = peakGyroDps_;
  result.orientationDeltaDeg = orientationDeltaDeg_;
  result.inactivityRatio = inactivityRatio;
  result.heartRateBpm = heartRateAtFall_;
  result.spo2Percent = spo2AtFall_;
  result.cardiacCrisis = cardiacCrisis_;
  result.fingerPresent = fingerDetected_;
  result.movementResumed = movementResumed_;
  result.fallType = currentFallType_;
  return result;
}

bool CrashDetector::crashConfirmed() const {
  const CrashEvidence currentEvidence = evidence();

  // GUARD 0 (Khoa tiep xuc quang hoc PPG):
  // Neu cam bien MAX30100 hoat dong ma khong phat hien tiep xuc da/ngon tay
  // -> Tuyet doi khong bao nga (chong cam lac ngoai khong khi hoac roi vat vo tri)
  if (!fingerDetected_) {
    return false;
  }

  // GUARD 0.5 (Bo loc chong rung lac tay / vay tay dao dong lien tuc - Shake Rejection):
  // Cu nga that chi co 1 cu va dap chinh. Neu co >= 3 dinh gia toc hoac doi chieu lac >= 3 lan -> Day la lac tay!
  if (impactSpikeCount_ >= 3 || gyroReversals_ >= 3) {
    return false;
  }

  // GUARD 1: Bat buoc phai co va dap toi thieu!
  // Neu co bien co tim mach: nguong va dap chi can >= 1.45g
  // Neu nga co hoc: bat buoc >= 2.10g
  const float minImpactReq = cardiacCrisis_ ? 1.45f : 2.10f;
  if (peakAccelerationG_ < minImpactReq) {
    return false;
  }

  // GUARD 2 (Triet tieu van dong thuong ngay & di bo / vung tay):
  if (currentEvidence.inactivityRatio < 0.20f) {
    return false;
  }

  // GUARD 3 (Kiem tra lat doi tu the co the bat buoc >= 30 do):
  if (orientationDeltaDeg_ < 30.0f && peakAccelerationG_ < 3.80f) {
    return false;
  }

  // GUARD 4 (Chong bao gia khi dat vong len ban):
  if (minAccelerationG_ > 0.55f && peakGyroDps_ < 320.0f && peakAccelerationG_ < 3.50f && currentEvidence.inactivityRatio >= 0.85f) {
    return false;
  }

  // GUARD 5 (Hop nhat nhip tim de loai bo bao gia khi nhip tim hoan toan binh thuong):
  // Khi nhip tim on dinh (55 - 100 BPM), co the khong bi soc hoang loan hay dau don cap
  // -> Yeu cau va dap co hoc ro ret (>= 2.70g), doi goc >= 38 do, va bat dong thuc su >= 30%
  if (normalHeartConfirmed_) {
    if (peakAccelerationG_ < 2.70f || orientationDeltaDeg_ < 38.0f || currentEvidence.inactivityRatio < 0.30f || currentEvidence.totalScore < 76) {
      return false;
    }
  }

  // 3 NHANH XAC NHAN TAI NAN:
  // Nhanh 1: Nga do Bien co Tim mach / Dot quy (Cardiac Syncope)
  if (cardiacCrisis_ && peakAccelerationG_ >= 1.45f && orientationDeltaDeg_ >= 28.0f) {
    const_cast<CrashDetector*>(this)->currentFallType_ = "FALL_CARDIAC";
    return true;
  }

  // Nhanh 2: Vap nga va dap manh (Dynamic Fall with Impact)
  const bool dynamicFall = (peakAccelerationG_ >= 2.45f) &&
                           (orientationDeltaDeg_ >= Config::ORIENTATION_THRESHOLD_DEG) &&
                           (currentEvidence.inactivityRatio >= 0.25f) &&
                           (currentEvidence.totalScore >= Config::CRASH_SCORE_THRESHOLD);
  if (dynamicFall) {
    const_cast<CrashDetector*>(this)->currentFallType_ = "FALL_IMPACT";
    return true;
  }

  // Nhanh 3: Nga sup xuong san bat dong (Soft Collapse Fall)
  const bool syncopeFall = (peakAccelerationG_ >= 2.05f) &&
                           (orientationDeltaDeg_ >= 35.0f) &&
                           (currentEvidence.inactivityRatio >= 0.45f) &&
                           (bestAiCrashProbability_ >= Config::AI_CRASH_PROBABILITY && currentEvidence.totalScore >= 72);
  if (syncopeFall) {
    const_cast<CrashDetector*>(this)->currentFallType_ = "FALL_SYNCOPE";
    return true;
  }

  return false;
}

uint8_t CrashDetector::weightedScore(float value01, uint8_t weight) const {
  const float bounded = SignalProcessing::clamp01(value01);
  return static_cast<uint8_t>(roundf(bounded * static_cast<float>(weight)));
}
