#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include "../VenomShared.hpp"
#include "EnvelopeADSR.hpp"
#include "Lowpass24.hpp"
#include "Oscillator.hpp"

namespace venom::dsp {

class SynthVoice {
public:
  void setSampleRate(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    for (auto& osc : oscillators_)
      osc.setSampleRate(sampleRate_);
    ampEnv_.setSampleRate(sampleRate_);
    filterEnv_.setSampleRate(sampleRate_);
    lowpass_.setSampleRate(sampleRate_);
  }

  void reset() {
    for (auto& osc : oscillators_)
      osc.reset();
    ampEnv_ = EnvelopeADSR{};
    ampEnv_.setSampleRate(sampleRate_);
    filterEnv_ = EnvelopeADSR{};
    filterEnv_.setSampleRate(sampleRate_);
    lowpass_.reset();
  }

  void setOscillator(uint32_t index, Waveform waveform, float detuneCents, float pitchSemitones) {
    if (index >= oscillators_.size())
      return;
    oscillators_[index].setWaveform(waveform);
    oscillators_[index].setDetuneCents(detuneCents);
    oscPitchSemitones_[index] = std::clamp(pitchSemitones, -12.0f, 12.0f);
    oscillators_[index].setPitchSemitones(oscPitchSemitones_[index]);
  }

  void setFilter(float cutoffHz, float resonance) {
    baseCutoffHz_ = std::clamp(cutoffHz, 20.0f, 20000.0f);
    lowpass_.setCutoff(baseCutoffHz_);
    lowpass_.setResonance(resonance);
  }

  void setAmpEnvelope(float attack, float decay, float sustain, float release, bool retrigger) {
    ampEnv_.setAttack(attack);
    ampEnv_.setDecay(decay);
    ampEnv_.setSustain(sustain);
    ampEnv_.setRelease(release);
    ampEnv_.setRetrigger(retrigger);
  }

  void setFilterEnvelope(float attack, float decay, float sustain, float release, bool retrigger, float envAmount) {
    filterEnv_.setAttack(attack);
    filterEnv_.setDecay(decay);
    filterEnv_.setSustain(sustain);
    filterEnv_.setRelease(release);
    filterEnv_.setRetrigger(retrigger);
    filterEnvAmount_ = std::clamp(envAmount, -1.0f, 1.0f);
  }

  void noteOn(uint8_t midiNote) {
    midiNote_ = midiNote;
    const double frequency = midiToHz(midiNote_);
    for (auto& osc : oscillators_)
      osc.setFrequency(frequency);
    ampEnv_.noteOn();
    filterEnv_.noteOn();
    active_ = true;
  }

  void noteOff() {
    ampEnv_.noteOff();
    filterEnv_.noteOff();
  }

  bool isActive() const { return active_; }

  float process(const std::array<float, kNumOscillators>& levels, float pitchModSemitones = 0.0f) {
    if (!active_)
      return 0.0f;

    for (uint32_t i = 0; i < oscillators_.size(); ++i)
      oscillators_[i].setPitchSemitones(oscPitchSemitones_[i] + pitchModSemitones);

    const float amp = ampEnv_.process();
    const float filterEnv = filterEnv_.process();

    const float cutoff = std::clamp(baseCutoffHz_ * (1.0f + filterEnvAmount_ * filterEnv), 20.0f, 20000.0f);
    lowpass_.setCutoff(cutoff);

    float sample = 0.0f;
    for (uint32_t i = 0; i < oscillators_.size(); ++i)
      sample += oscillators_[i].render() * levels[i];

    sample = lowpass_.process(sample);
    sample *= amp;

    if (!ampEnv_.isActive())
      active_ = false;

    return sample;
  }

private:
  static double midiToHz(uint8_t note) {
    return 440.0 * std::pow(2.0, (static_cast<int>(note) - 69) / 12.0);
  }

  double sampleRate_ = 44100.0;
  std::array<Oscillator, kNumOscillators> oscillators_{};
  EnvelopeADSR ampEnv_{};
  EnvelopeADSR filterEnv_{};
  Lowpass24 lowpass_{};
  uint8_t midiNote_ = 69;
  bool active_ = false;
  float baseCutoffHz_ = 1200.0f;
  float filterEnvAmount_ = 0.0f;
  std::array<float, kNumOscillators> oscPitchSemitones_{};
};

} // namespace venom::dsp
