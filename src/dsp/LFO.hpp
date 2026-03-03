#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace venom::dsp {

class LFO {
public:
  enum class Waveform : uint8_t {
    Sine = 0,
    Saw,
    Square,
    Triangle,
    SampleAndHold,
  };

  void setSampleRate(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    updateDelta();
  }

  void setRate(float rateHz) {
    rateHz_ = std::clamp(rateHz, 0.1f, 20.0f);
    updateDelta();
  }

  void setWaveform(Waveform wf) { waveform_ = wf; }

  void setWaveform(uint32_t index) {
    waveform_ = static_cast<Waveform>(std::min(index, 4u));
  }

  void reset() {
    phase_ = 0.0;
    shValue_ = 0.0f;
  }

  float process() {
    float value = 0.0f;
    const float p = static_cast<float>(phase_);

    switch (waveform_) {
      case Waveform::Sine: {
        constexpr float kTwoPi = 6.28318530718f;
        value = std::sin(kTwoPi * p);
        break;
      }

      case Waveform::Saw:
        // Bipolar saw: -1 at phase=0, rises to +1 at phase=1
        value = 2.0f * p - 1.0f;
        break;

      case Waveform::Square:
        value = (p < 0.5f) ? 1.0f : -1.0f;
        break;

      case Waveform::Triangle:
        // Triangle: 0→+1→0→-1→0 over one cycle
        value = 4.0f * std::abs(p - 0.5f) - 1.0f;
        break;

      case Waveform::SampleAndHold: {
        value = shValue_;
        break;
      }
    }

    // Advance phase
    const double oldPhase = phase_;
    phase_ += phaseDelta_;
    if (phase_ >= 1.0) {
      phase_ -= 1.0;
      // S&H: latch new random value on phase wrap
      if (waveform_ == Waveform::SampleAndHold) {
        shValue_ = nextRandom();
      }
    }
    (void)oldPhase;

    return value;
  }

private:
  void updateDelta() {
    phaseDelta_ = static_cast<double>(rateHz_) / sampleRate_;
  }

  // RT-safe pseudo-random: xorshift32, returns bipolar [-1, +1]
  float nextRandom() {
    rngState_ ^= rngState_ << 13;
    rngState_ ^= rngState_ >> 17;
    rngState_ ^= rngState_ << 5;
    // Map uint32 to [-1, 1]
    return static_cast<float>(rngState_) / 2147483648.0f - 1.0f;
  }

  double sampleRate_ = 44100.0;
  double phase_ = 0.0;
  double phaseDelta_ = 3.0 / 44100.0;
  float rateHz_ = 3.0f;
  Waveform waveform_ = Waveform::Sine;

  // Sample & Hold state
  float shValue_ = 0.0f;
  uint32_t rngState_ = 0x12345678u;
};

} // namespace venom::dsp
