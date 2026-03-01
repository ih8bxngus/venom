#pragma once

#include <algorithm>
#include <cmath>
#include "../VenomShared.hpp"

namespace venom::dsp {

class Oscillator {
public:
  void setSampleRate(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    updatePhaseDelta();
  }

  void setWaveform(Waveform waveform) { waveform_ = waveform; }

  void setFrequency(double frequencyHz) {
    baseFrequencyHz_ = std::clamp(frequencyHz, 20.0, 20000.0);
    updatePhaseDelta();
  }

  void setDetuneCents(float cents) {
    detuneCents_ = cents;
    updatePhaseDelta();
  }

  void setPitchSemitones(float semitones) {
    pitchSemitones_ = std::clamp(semitones, -12.0f, 12.0f);
    updatePhaseDelta();
  }

  void reset() { phase_ = 0.0; }

  float render() {
    const float value = renderWaveform();
    phase_ += phaseDelta_;
    if (phase_ >= 1.0)
      phase_ -= 1.0;
    return value;
  }

private:
  float renderWaveform() const {
    constexpr float pi = 3.14159265359f;
    const float phase = static_cast<float>(phase_);

    switch (waveform_) {
      case Waveform::Sine:
        return std::sin(2.0f * pi * phase);
      case Waveform::Saw:
        return (2.0f * phase) - 1.0f;
      case Waveform::Square:
        return phase < 0.5f ? 1.0f : -1.0f;
      case Waveform::Triangle:
        return 1.0f - 4.0f * std::fabs(phase - 0.5f);
      default:
        return 0.0f;
    }
  }

  void updatePhaseDelta() {
    const double semitoneRatio = std::pow(2.0, pitchSemitones_ / 12.0);
    const double centRatio = std::pow(2.0, detuneCents_ / 1200.0);
    const double frequencyHz = baseFrequencyHz_ * semitoneRatio * centRatio;
    phaseDelta_ = frequencyHz / sampleRate_;
  }

  double sampleRate_ = 44100.0;
  double baseFrequencyHz_ = 110.0;
  double phase_ = 0.0;
  double phaseDelta_ = 110.0 / 44100.0;
  float detuneCents_ = 0.0f;
  float pitchSemitones_ = 0.0f;
  Waveform waveform_ = Waveform::Saw;
};

} // namespace venom::dsp
