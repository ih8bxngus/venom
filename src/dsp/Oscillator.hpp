#pragma once

#include <algorithm>
#include <cmath>
#include "../VenomShared.hpp"
#include "Wavetable.hpp"

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
    pitchSemitones_ = std::clamp(semitones, -48.0f, 48.0f);
    updatePhaseDelta();
  }

  void setWavetable(const Wavetable* wt) { wavetable_ = wt; }
  void setWavetablePosition(float pos) { wavetablePosition_ = std::clamp(pos, 0.0f, 1.0f); }

  void reset() { phase_ = 0.0; lastOutput_ = 0.0f; }

  // Standard render (no phase modulation)
  float render() {
    lastOutput_ = renderWaveform(phase_);
    phase_ += phaseDelta_;
    if (phase_ >= 1.0)
      phase_ -= 1.0;
    return lastOutput_;
  }

  // Render with phase modulation (for FM synthesis)
  // phaseModRadians is added to the internal phase before waveform lookup
  float render(float phaseModRadians) {
    constexpr double kInvTwoPi = 1.0 / 6.28318530718;
    double modulatedPhase = phase_ + static_cast<double>(phaseModRadians) * kInvTwoPi;
    // Wrap to [0, 1)
    modulatedPhase = modulatedPhase - std::floor(modulatedPhase);
    lastOutput_ = renderWaveform(modulatedPhase);
    phase_ += phaseDelta_;
    if (phase_ >= 1.0)
      phase_ -= 1.0;
    return lastOutput_;
  }

  float getLastOutput() const { return lastOutput_; }

private:
  // PolyBLEP anti-aliasing: polynomial band-limited step correction
  static float polyBLEP(double t, double dt) {
    if (t < dt) {
      const double tn = t / dt;
      return static_cast<float>(tn + tn - tn * tn - 1.0);
    }
    if (t > 1.0 - dt) {
      const double tn = (t - 1.0) / dt;
      return static_cast<float>(tn * tn + tn + tn + 1.0);
    }
    return 0.0f;
  }

  float renderWaveform(double phaseVal) const {
    constexpr float pi = 3.14159265359f;
    const float phase = static_cast<float>(phaseVal);
    const double dt = phaseDelta_;

    switch (waveform_) {
      case Waveform::Sine:
        return std::sin(2.0f * pi * phase);

      case Waveform::Saw: {
        // Naive saw with polyBLEP correction at discontinuity
        float value = (2.0f * phase) - 1.0f;
        value -= polyBLEP(phaseVal, dt);
        return value;
      }

      case Waveform::Square: {
        // Naive square with polyBLEP correction at both edges
        float value = phase < 0.5f ? 1.0f : -1.0f;
        value += polyBLEP(phaseVal, dt);
        value -= polyBLEP(std::fmod(phaseVal + 0.5, 1.0), dt);
        return value;
      }

      case Waveform::Triangle:
        return 1.0f - 4.0f * std::fabs(phase - 0.5f);

      case Waveform::Wavetable:
        if (wavetable_ != nullptr)
          return wavetable_->sample(wavetablePosition_, phase);
        return 0.0f;

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

  // FM synthesis
  float lastOutput_ = 0.0f;

  // Wavetable support
  const Wavetable* wavetable_ = nullptr;
  float wavetablePosition_ = 0.0f;
};

} // namespace venom::dsp
