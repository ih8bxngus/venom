#pragma once

#include <algorithm>
#include <cmath>

namespace venom::dsp {

class LFO {
public:
  void setSampleRate(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    updateDelta();
  }

  void setRate(float rateHz) {
    rateHz_ = std::clamp(rateHz, 0.1f, 20.0f);
    updateDelta();
  }

  void reset() { phase_ = 0.0; }

  float process() {
    constexpr float kTwoPi = 6.28318530718f;
    const float value = std::sin(kTwoPi * static_cast<float>(phase_));
    phase_ += phaseDelta_;
    if (phase_ >= 1.0)
      phase_ -= 1.0;
    return value;
  }

private:
  void updateDelta() {
    phaseDelta_ = static_cast<double>(rateHz_) / sampleRate_;
  }

  double sampleRate_ = 44100.0;
  double phase_ = 0.0;
  double phaseDelta_ = 3.0 / 44100.0;
  float rateHz_ = 3.0f;
};

} // namespace venom::dsp
