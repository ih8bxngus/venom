#pragma once

#include <algorithm>

namespace venom::dsp {

class EnvelopeADSR {
public:
  void setSampleRate(double sampleRate) { sampleRate_ = std::max(1.0, sampleRate); }

  void setAttack(float seconds) { attack_ = std::max(0.001f, seconds); }
  void setDecay(float seconds) { decay_ = std::max(0.001f, seconds); }
  void setSustain(float level) { sustain_ = std::clamp(level, 0.0f, 1.0f); }
  void setRelease(float seconds) { release_ = std::max(0.001f, seconds); }

  void setRetrigger(bool retrigger) { retrigger_ = retrigger; }

  void noteOn() {
    if (retrigger_)
      value_ = 0.0f;
    stage_ = Stage::Attack;
  }

  void noteOff() {
    releaseStart_ = value_;
    stage_ = Stage::Release;
  }

  bool isActive() const { return stage_ != Stage::Idle; }

  float process() {
    switch (stage_) {
      case Stage::Idle:
        value_ = 0.0f;
        break;
      case Stage::Attack:
        value_ += 1.0f / (attack_ * static_cast<float>(sampleRate_));
        if (value_ >= 1.0f) {
          value_ = 1.0f;
          stage_ = Stage::Decay;
        }
        break;
      case Stage::Decay:
        value_ -= (1.0f - sustain_) / (decay_ * static_cast<float>(sampleRate_));
        if (value_ <= sustain_) {
          value_ = sustain_;
          stage_ = Stage::Sustain;
        }
        break;
      case Stage::Sustain:
        value_ = sustain_;
        break;
      case Stage::Release:
        value_ -= releaseStart_ / (release_ * static_cast<float>(sampleRate_));
        if (value_ <= 0.0f) {
          value_ = 0.0f;
          stage_ = Stage::Idle;
        }
        break;
    }

    return value_;
  }

private:
  enum class Stage { Idle, Attack, Decay, Sustain, Release };

  Stage stage_ = Stage::Idle;
  double sampleRate_ = 44100.0;
  float attack_ = 0.01f;
  float decay_ = 0.08f;
  float sustain_ = 0.8f;
  float release_ = 0.1f;
  float value_ = 0.0f;
  float releaseStart_ = 0.0f;
  bool retrigger_ = true;
};

} // namespace venom::dsp
