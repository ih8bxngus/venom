#pragma once

#include <algorithm>
#include <cmath>

namespace venom::dsp {

class EnvelopeADSR {
public:
  void setSampleRate(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    recomputeCoeffs();
  }

  void setAttack(float seconds) {
    attack_ = std::max(0.001f, seconds);
    computeAttackCoeff();
  }

  void setDecay(float seconds) {
    decay_ = std::max(0.001f, seconds);
    computeDecayCoeff();
  }

  void setSustain(float level) { sustain_ = std::clamp(level, 0.0f, 1.0f); }

  void setRelease(float seconds) {
    release_ = std::max(0.001f, seconds);
    computeReleaseCoeff();
  }

  void setRetrigger(bool retrigger) { retrigger_ = retrigger; }

  void noteOn() {
    if (retrigger_)
      value_ = 0.0f;
    stage_ = Stage::Attack;
  }

  void noteOff() {
    stage_ = Stage::Release;
  }

  bool isActive() const { return stage_ != Stage::Idle; }

  float process() {
    switch (stage_) {
      case Stage::Idle:
        value_ = 0.0f;
        break;

      case Stage::Attack: {
        // Exponential rise toward overshoot target for natural curve shape
        value_ = kAttackTarget + (value_ - kAttackTarget) * attackCoeff_;
        if (value_ >= 1.0f) {
          value_ = 1.0f;
          stage_ = Stage::Decay;
        }
        break;
      }

      case Stage::Decay: {
        // Exponential decay toward sustain level
        value_ = sustain_ + (value_ - sustain_) * decayCoeff_;
        if (value_ - sustain_ < kMinLevel) {
          value_ = sustain_;
          stage_ = Stage::Sustain;
        }
        break;
      }

      case Stage::Sustain:
        value_ = sustain_;
        break;

      case Stage::Release: {
        // Exponential decay toward zero
        value_ *= releaseCoeff_;
        if (value_ < kMinLevel) {
          value_ = 0.0f;
          stage_ = Stage::Idle;
        }
        break;
      }
    }

    return value_;
  }

private:
  enum class Stage { Idle, Attack, Decay, Sustain, Release };

  static constexpr float kMinLevel = 1.0e-5f;
  static constexpr float kAttackTarget = 1.3f;

  void recomputeCoeffs() {
    computeAttackCoeff();
    computeDecayCoeff();
    computeReleaseCoeff();
  }

  void computeAttackCoeff() {
    const float samples = attack_ * static_cast<float>(sampleRate_);
    attackCoeff_ = std::exp(-std::log(kAttackTarget / (kAttackTarget - 1.0f)) / samples);
  }

  void computeDecayCoeff() {
    const float samples = decay_ * static_cast<float>(sampleRate_);
    decayCoeff_ = std::exp(-std::log(1.0f / kMinLevel) / samples);
  }

  void computeReleaseCoeff() {
    const float samples = release_ * static_cast<float>(sampleRate_);
    releaseCoeff_ = std::exp(-std::log(1.0f / kMinLevel) / samples);
  }

  Stage stage_ = Stage::Idle;
  double sampleRate_ = 44100.0;
  float attack_ = 0.01f;
  float decay_ = 0.08f;
  float sustain_ = 0.8f;
  float release_ = 0.1f;
  float value_ = 0.0f;
  bool retrigger_ = true;

  float attackCoeff_ = 0.99f;
  float decayCoeff_ = 0.99f;
  float releaseCoeff_ = 0.99f;
};

} // namespace venom::dsp
