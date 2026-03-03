#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include "../VenomShared.hpp"
#include "EnvelopeADSR.hpp"
#include "Lowpass24.hpp"
#include "UnisonOscBank.hpp"

namespace venom::dsp {

class SynthVoice {
public:
  void setSampleRate(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    for (auto& bank : oscBanks_)
      bank.setSampleRate(sampleRate_);
    ampEnv_.setSampleRate(sampleRate_);
    filterEnv_.setSampleRate(sampleRate_);
    lowpassL_.setSampleRate(sampleRate_);
    lowpassR_.setSampleRate(sampleRate_);
  }

  void reset() {
    for (auto& bank : oscBanks_)
      bank.reset();
    ampEnv_ = EnvelopeADSR{};
    ampEnv_.setSampleRate(sampleRate_);
    filterEnv_ = EnvelopeADSR{};
    filterEnv_.setSampleRate(sampleRate_);
    lowpassL_.reset();
    lowpassR_.reset();
  }

  void setOscillator(uint32_t index, Waveform waveform, float detuneCents, float pitchSemitones) {
    if (index >= oscBanks_.size())
      return;
    oscBanks_[index].setWaveform(waveform);
    oscBanks_[index].setDetuneCents(detuneCents);
    oscPitchSemitones_[index] = std::clamp(pitchSemitones, -48.0f, 48.0f);
    oscBanks_[index].setPitchSemitones(oscPitchSemitones_[index]);
  }

  void setFilter(float cutoffHz, float resonance) {
    baseCutoffHz_ = std::clamp(cutoffHz, 20.0f, 20000.0f);
    lowpassL_.setCutoff(baseCutoffHz_);
    lowpassL_.setResonance(resonance);
    lowpassR_.setCutoff(baseCutoffHz_);
    lowpassR_.setResonance(resonance);
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

  void setUnisonCount(uint32_t count) {
    for (auto& bank : oscBanks_)
      bank.setUnisonCount(count);
  }

  void setUnisonDetune(float cents) {
    for (auto& bank : oscBanks_)
      bank.setUnisonDetune(cents);
  }

  void setUnisonSpread(float spread) {
    for (auto& bank : oscBanks_)
      bank.setUnisonSpread(spread);
  }

  void setWavetable(const Wavetable* wt) {
    for (auto& bank : oscBanks_)
      bank.setWavetable(wt);
  }

  void setWavetable(uint32_t oscIndex, const Wavetable* wt) {
    if (oscIndex < oscBanks_.size())
      oscBanks_[oscIndex].setWavetable(wt);
  }

  void setFMAmounts(float fm2to1, float fm3to1, float fm3to2) {
    fmAmount2to1_ = fm2to1;
    fmAmount3to1_ = fm3to1;
    fmAmount3to2_ = fm3to2;
  }

  void setWavetablePosition(uint32_t oscIndex, float pos) {
    if (oscIndex < oscBanks_.size())
      oscBanks_[oscIndex].setWavetablePosition(pos);
  }

  void noteOn(uint8_t midiNote) {
    midiNote_ = midiNote;
    const double frequency = midiToHz(midiNote_);
    for (auto& bank : oscBanks_)
      bank.setFrequency(frequency);
    ampEnv_.noteOn();
    filterEnv_.noteOn();
    active_ = true;
  }

  void noteOff() {
    ampEnv_.noteOff();
    filterEnv_.noteOff();
  }

  bool isActive() const { return active_; }
  uint8_t getMidiNote() const { return midiNote_; }

  StereoSample process(const std::array<float, kNumOscillators>& levels, float pitchModSemitones = 0.0f) {
    if (!active_)
      return {0.0f, 0.0f};

    for (uint32_t i = 0; i < oscBanks_.size(); ++i)
      oscBanks_[i].setPitchSemitones(oscPitchSemitones_[i] + pitchModSemitones);

    const float amp = ampEnv_.process();
    const float filterEnv = filterEnv_.process();

    const float cutoff = std::clamp(baseCutoffHz_ * (1.0f + filterEnvAmount_ * filterEnv), 20.0f, 20000.0f);
    lowpassL_.setCutoff(cutoff);
    lowpassR_.setCutoff(cutoff);

    // === FM Rendering Order: Osc3 (unmodulated) → Osc2 → Osc1 ===
    // FM amount 0-1 maps to 0-8π radians of phase modulation depth
    constexpr float kFMScale = 8.0f * 3.14159265359f;

    // Step 1: Render Osc3 (never modulated — it's the top of the chain)
    const auto osc3out = oscBanks_[2].render();
    const float osc3Mod = oscBanks_[2].getCenterOutput();

    // Step 2: Render Osc2 (modulated by Osc3)
    const float mod2 = osc3Mod * fmAmount3to2_ * kFMScale;
    const auto osc2out = (mod2 != 0.0f) ? oscBanks_[1].render(mod2) : oscBanks_[1].render();
    const float osc2Mod = oscBanks_[1].getCenterOutput();

    // Step 3: Render Osc1 (modulated by Osc2 + Osc3)
    const float mod1 = osc2Mod * fmAmount2to1_ * kFMScale
                      + osc3Mod * fmAmount3to1_ * kFMScale;
    const auto osc1out = (mod1 != 0.0f) ? oscBanks_[0].render(mod1) : oscBanks_[0].render();

    // Mix all oscillators
    float mixL = osc1out.left  * levels[0] + osc2out.left  * levels[1] + osc3out.left  * levels[2];
    float mixR = osc1out.right * levels[0] + osc2out.right * levels[1] + osc3out.right * levels[2];

    // Stereo filter
    mixL = lowpassL_.process(mixL);
    mixR = lowpassR_.process(mixR);

    // Apply amp envelope
    mixL *= amp;
    mixR *= amp;

    if (!ampEnv_.isActive())
      active_ = false;

    return {mixL, mixR};
  }

private:
  static double midiToHz(uint8_t note) {
    return 440.0 * std::pow(2.0, (static_cast<int>(note) - 69) / 12.0);
  }

  double sampleRate_ = 44100.0;
  std::array<UnisonOscBank, kNumOscillators> oscBanks_{};
  EnvelopeADSR ampEnv_{};
  EnvelopeADSR filterEnv_{};
  Lowpass24 lowpassL_{};
  Lowpass24 lowpassR_{};
  uint8_t midiNote_ = 69;
  bool active_ = false;
  float baseCutoffHz_ = 1200.0f;
  float filterEnvAmount_ = 0.0f;
  std::array<float, kNumOscillators> oscPitchSemitones_{};

  // FM synthesis
  float fmAmount2to1_ = 0.0f;
  float fmAmount3to1_ = 0.0f;
  float fmAmount3to2_ = 0.0f;
};

} // namespace venom::dsp
