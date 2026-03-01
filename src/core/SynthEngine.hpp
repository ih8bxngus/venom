#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "../VenomShared.hpp"
#include "../dsp/LFO.hpp"
#include "../dsp/SynthVoice.hpp"

namespace venom::core {

struct MidiEvent {
  uint32_t frame = 0;
  uint8_t status = 0;
  uint8_t data1 = 0;
  uint8_t data2 = 0;
};

class SynthEngine {
public:
  void setSampleRate(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    voice_.setSampleRate(sampleRate_);
    lfo_.setSampleRate(sampleRate_);
  }

  void setParameters(const std::array<float, kParameterCount>& params) {
    params_ = params;

    for (uint32_t i = 0; i < kNumOscillators; ++i) {
      const uint32_t base = kParamOsc1Wave + (i * 4);
      auto wf = static_cast<Waveform>(static_cast<int>(std::clamp(params_[base], 0.0f, 3.0f)));
      voice_.setOscillator(i, wf, params_[base + 1], params_[base + 2]);
      oscLevels_[i] = std::clamp(params_[base + 3], 0.0f, 1.0f);
    }

    voice_.setFilter(params_[kParamFilterCutoff], params_[kParamFilterResonance]);
    const bool retrigger = params_[kParamEnvRetrigger] > 0.5f;

    voice_.setAmpEnvelope(params_[kParamAmpAttack], params_[kParamAmpDecay], params_[kParamAmpSustain], params_[kParamAmpRelease], retrigger);
    voice_.setFilterEnvelope(params_[kParamFilterAttack], params_[kParamFilterDecay], params_[kParamFilterSustain], params_[kParamFilterRelease], retrigger, params_[kParamFilterEnvAmount]);

    lfo_.setRate(params_[kParamLfoRate]);
  }

  void noteOn(uint8_t note, uint8_t velocity) {
    const float velocityNorm = std::clamp(velocity / 127.0f, 0.0f, 1.0f);
    noteVelocity_[note] = velocityNorm;

    removeHeldNote(note);
    pushHeldNote(note);

    lastVelocity_ = velocityNorm;
    currentNote_ = note;
    voice_.noteOn(note);
  }

  void noteOff(uint8_t note) {
    const bool removed = removeHeldNote(note);
    noteVelocity_[note] = 0.0f;

    if (!removed)
      return;

    if (heldNotesCount_ == 0) {
      currentNote_ = -1;
      voice_.noteOff();
      return;
    }

    const uint8_t fallbackNote = heldNotes_[heldNotesCount_ - 1];
    currentNote_ = static_cast<int>(fallbackNote);
    lastVelocity_ = noteVelocity_[fallbackNote];
    voice_.noteOn(fallbackNote);
  }

  void allNotesOff() {
    heldNotesCount_ = 0;
    currentNote_ = -1;
    noteVelocity_.fill(0.0f);
    voice_.noteOff();
  }

  void process(const MidiEvent* events, uint32_t eventCount, float* left, float* right, uint32_t frames) {
    if (left == nullptr || right == nullptr)
      return;

    uint32_t eventIndex = 0;
    for (uint32_t frame = 0; frame < frames; ++frame) {
      while (eventIndex < eventCount && events[eventIndex].frame == frame) {
        handleMidiEvent(events[eventIndex]);
        ++eventIndex;
      }

      const float lfo = lfo_.process() * std::clamp(params_[kParamLfoDepth], 0.0f, 1.0f);
      float pitchMod = 0.0f;

      if (static_cast<int>(params_[kParamLfoTarget]) == 0) {
        pitchMod = lfo * 2.0f;
      } else {
        const float cutoff = std::clamp(params_[kParamFilterCutoff] * (1.0f + lfo), 20.0f, 20000.0f);
        voice_.setFilter(cutoff, params_[kParamFilterResonance]);
      }

      float sample = voice_.process(oscLevels_, pitchMod);
      sample *= lastVelocity_;
      sample *= (1.0f + params_[kParamDrive] * 8.0f);
      sample = std::tanh(sample);
      sample *= std::clamp(params_[kParamMasterVolume], 0.0f, 1.0f);

      left[frame] = sample;
      right[frame] = sample;
    }
  }

private:
  void handleMidiEvent(const MidiEvent& event) {
    const uint8_t type = event.status & 0xF0;
    if (type == 0x90 && event.data2 > 0) {
      noteOn(event.data1, event.data2);
    } else if (type == 0x80 || (type == 0x90 && event.data2 == 0)) {
      noteOff(event.data1);
    } else if (type == 0xB0) {
      if (event.data1 == 120 || event.data1 == 123)
        allNotesOff();
    }
  }

  void pushHeldNote(uint8_t note) {
    if (heldNotesCount_ < heldNotes_.size()) {
      heldNotes_[heldNotesCount_++] = note;
      return;
    }

    for (size_t i = 1; i < heldNotes_.size(); ++i)
      heldNotes_[i - 1] = heldNotes_[i];

    heldNotes_[heldNotes_.size() - 1] = note;
  }

  bool removeHeldNote(uint8_t note) {
    for (size_t i = 0; i < heldNotesCount_; ++i) {
      if (heldNotes_[i] != note)
        continue;
      for (size_t j = i + 1; j < heldNotesCount_; ++j)
        heldNotes_[j - 1] = heldNotes_[j];
      --heldNotesCount_;
      return true;
    }
    return false;
  }

  double sampleRate_ = 44100.0;
  std::array<float, kParameterCount> params_{};
  std::array<float, kNumOscillators> oscLevels_{0.6f, 0.4f, 0.35f};
  std::array<float, 128> noteVelocity_{};
  std::array<uint8_t, 16> heldNotes_{};
  size_t heldNotesCount_ = 0;
  int currentNote_ = -1;

  dsp::SynthVoice voice_{};
  dsp::LFO lfo_{};
  float lastVelocity_ = 0.0f;
};

} // namespace venom::core
