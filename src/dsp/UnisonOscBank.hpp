#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#include "../VenomShared.hpp"
#include "Oscillator.hpp"

namespace venom::dsp {

struct StereoSample {
  float left = 0.0f;
  float right = 0.0f;
};

class UnisonOscBank {
public:
  void setSampleRate(double sampleRate) {
    for (auto& osc : oscillators_)
      osc.setSampleRate(sampleRate);
  }

  void setWaveform(Waveform waveform) {
    for (auto& osc : oscillators_)
      osc.setWaveform(waveform);
  }

  void setFrequency(double frequencyHz) {
    baseFrequencyHz_ = frequencyHz;
    for (auto& osc : oscillators_)
      osc.setFrequency(frequencyHz);
  }

  void setDetuneCents(float cents) {
    baseDetuneCents_ = cents;
    updateVoiceDetune();
  }

  void setPitchSemitones(float semitones) {
    for (auto& osc : oscillators_)
      osc.setPitchSemitones(semitones);
  }

  void setWavetable(const Wavetable* wt) {
    for (auto& osc : oscillators_)
      osc.setWavetable(wt);
  }

  void setWavetablePosition(float pos) {
    for (auto& osc : oscillators_)
      osc.setWavetablePosition(pos);
  }

  void setUnisonCount(uint32_t count) {
    unisonCount_ = std::clamp(count, 1u, kMaxUnisonVoices);
    updateVoiceDetune();
    updateGain();
  }

  void setUnisonDetune(float cents) {
    unisonDetuneCents_ = std::clamp(cents, 0.0f, 100.0f);
    updateVoiceDetune();
  }

  void setUnisonSpread(float spread) {
    unisonSpread_ = std::clamp(spread, 0.0f, 1.0f);
    updatePanning();
  }

  void reset() {
    for (auto& osc : oscillators_)
      osc.reset();
  }

  StereoSample render() {
    if (unisonCount_ == 1) {
      // No unison: mono center output
      const float s = oscillators_[0].render();
      return {s, s};
    }

    float left = 0.0f;
    float right = 0.0f;

    for (uint32_t i = 0; i < unisonCount_; ++i) {
      const float s = oscillators_[i].render();
      left  += s * panL_[i];
      right += s * panR_[i];
    }

    left  *= gain_;
    right *= gain_;

    return {left, right};
  }

  // Render with phase modulation applied to all unison voices (FM synthesis)
  StereoSample render(float phaseModRadians) {
    if (unisonCount_ == 1) {
      const float s = oscillators_[0].render(phaseModRadians);
      return {s, s};
    }

    float left = 0.0f;
    float right = 0.0f;

    for (uint32_t i = 0; i < unisonCount_; ++i) {
      const float s = oscillators_[i].render(phaseModRadians);
      left  += s * panL_[i];
      right += s * panR_[i];
    }

    left  *= gain_;
    right *= gain_;

    return {left, right};
  }

  // Get center oscillator's last output for use as FM modulator signal
  float getCenterOutput() const {
    return oscillators_[0].getLastOutput();
  }

private:
  void updateVoiceDetune() {
    if (unisonCount_ <= 1) {
      oscillators_[0].setDetuneCents(baseDetuneCents_);
      return;
    }

    // Spread voices evenly from -unisonDetuneCents_ to +unisonDetuneCents_
    for (uint32_t i = 0; i < unisonCount_; ++i) {
      const float t = static_cast<float>(i) / static_cast<float>(unisonCount_ - 1); // 0..1
      const float voiceDetune = unisonDetuneCents_ * (2.0f * t - 1.0f); // -spread..+spread
      oscillators_[i].setDetuneCents(baseDetuneCents_ + voiceDetune);
    }
  }

  void updatePanning() {
    if (unisonCount_ <= 1) {
      panL_[0] = 1.0f;
      panR_[0] = 1.0f;
      return;
    }

    constexpr float kHalfPi = 1.57079632679f;

    for (uint32_t i = 0; i < unisonCount_; ++i) {
      const float t = static_cast<float>(i) / static_cast<float>(unisonCount_ - 1); // 0..1
      // Pan position: 0=left, 0.5=center, 1=right, scaled by spread
      const float pan = 0.5f + unisonSpread_ * (t - 0.5f);

      // Constant-power panning
      panL_[i] = std::cos(pan * kHalfPi);
      panR_[i] = std::sin(pan * kHalfPi);
    }
  }

  void updateGain() {
    // Normalize gain: 1/sqrt(count)
    gain_ = 1.0f / std::sqrt(static_cast<float>(unisonCount_));
    updatePanning();
  }

  std::array<Oscillator, kMaxUnisonVoices> oscillators_{};

  double baseFrequencyHz_ = 110.0;
  float baseDetuneCents_ = 0.0f;

  uint32_t unisonCount_ = 1;
  float unisonDetuneCents_ = 15.0f;
  float unisonSpread_ = 0.5f;

  float gain_ = 1.0f;
  std::array<float, kMaxUnisonVoices> panL_{};
  std::array<float, kMaxUnisonVoices> panR_{};
};

} // namespace venom::dsp
