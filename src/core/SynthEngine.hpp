#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "../VenomShared.hpp"
#include "../dsp/LFO.hpp"
#include "../dsp/SynthVoice.hpp"
#include "../dsp/Wavetable.hpp"

namespace venom::core {

struct MidiEvent {
  uint32_t frame = 0;
  uint8_t status = 0;
  uint8_t data1 = 0;
  uint8_t data2 = 0;
};

class SynthEngine {
public:
  SynthEngine() {
    // Share wavetable across all voices
    for (auto& slot : voices_)
      slot.voice.setWavetable(&wavetable_);
  }

  void setSampleRate(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    for (auto& slot : voices_)
      slot.voice.setSampleRate(sampleRate_);
    lfo_.setSampleRate(sampleRate_);
  }

  void setParameters(const std::array<float, kParameterCount>& params) {
    params_ = params;

    polyMode_ = params_[kParamPolyMode] > 0.5f;
    const uint32_t unisonCount = static_cast<uint32_t>(std::clamp(params_[kParamUnisonCount], 1.0f, 7.0f));
    const float unisonDetune = std::clamp(params_[kParamUnisonDetune], 0.0f, 100.0f);
    const float unisonSpread = std::clamp(params_[kParamUnisonSpread], 0.0f, 1.0f);

    for (auto& slot : voices_) {
      for (uint32_t i = 0; i < kNumOscillators; ++i) {
        const uint32_t base = kParamOsc1Wave + (i * 4);
        auto wf = static_cast<Waveform>(static_cast<int>(std::clamp(params_[base], 0.0f, 4.0f)));
        slot.voice.setOscillator(i, wf, params_[base + 1], params_[base + 2]);
        oscLevels_[i] = std::clamp(params_[base + 3], 0.0f, 1.0f);
      }

      slot.voice.setFilter(params_[kParamFilterCutoff], params_[kParamFilterResonance]);
      const bool retrigger = params_[kParamEnvRetrigger] > 0.5f;

      slot.voice.setAmpEnvelope(
        params_[kParamAmpAttack], params_[kParamAmpDecay],
        params_[kParamAmpSustain], params_[kParamAmpRelease], retrigger);

      slot.voice.setFilterEnvelope(
        params_[kParamFilterAttack], params_[kParamFilterDecay],
        params_[kParamFilterSustain], params_[kParamFilterRelease],
        retrigger, params_[kParamFilterEnvAmount]);

      slot.voice.setUnisonCount(unisonCount);
      slot.voice.setUnisonDetune(unisonDetune);
      slot.voice.setUnisonSpread(unisonSpread);

      // Wavetable positions per oscillator
      slot.voice.setWavetablePosition(0, params_[kParamOsc1WavePos]);
      slot.voice.setWavetablePosition(1, params_[kParamOsc2WavePos]);
      slot.voice.setWavetablePosition(2, params_[kParamOsc3WavePos]);

      // FM synthesis amounts
      slot.voice.setFMAmounts(
        std::clamp(params_[kParamFM2to1], 0.0f, 1.0f),
        std::clamp(params_[kParamFM3to1], 0.0f, 1.0f),
        std::clamp(params_[kParamFM3to2], 0.0f, 1.0f));
    }

    lfo_.setRate(params_[kParamLfoRate]);
    lfo_.setWaveform(static_cast<uint32_t>(std::clamp(params_[kParamLfoWave], 0.0f, 4.0f)));
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

      // Advance global age for voice stealing
      ++globalAge_;

      const float lfoVal = lfo_.process() * std::clamp(params_[kParamLfoDepth], 0.0f, 1.0f);
      float pitchMod = 0.0f;
      bool lfoToFilter = false;

      if (static_cast<int>(params_[kParamLfoTarget]) == 0) {
        pitchMod = lfoVal * 2.0f;
      } else {
        lfoToFilter = true;
      }

      float sumL = 0.0f;
      float sumR = 0.0f;
      float activeVoiceCount = 0.0f;

      const uint32_t voiceCount = polyMode_ ? kMaxPolyVoices : 1u;

      for (uint32_t v = 0; v < voiceCount; ++v) {
        auto& slot = voices_[v];
        if (!slot.voice.isActive())
          continue;

        if (lfoToFilter) {
          const float cutoff = std::clamp(params_[kParamFilterCutoff] * (1.0f + lfoVal), 20.0f, 20000.0f);
          slot.voice.setFilter(cutoff, params_[kParamFilterResonance]);
        }

        auto sample = slot.voice.process(oscLevels_, pitchMod);
        const float vel = slot.velocity;
        sumL += sample.left * vel;
        sumR += sample.right * vel;
        activeVoiceCount += 1.0f;
      }

      // Normalize by sqrt of active voice count to prevent clipping
      const float voiceNorm = 1.0f / std::max(1.0f, std::sqrt(activeVoiceCount));
      sumL *= voiceNorm;
      sumR *= voiceNorm;

      // Drive + saturation + master volume
      const float driveGain = 1.0f + params_[kParamDrive] * 4.0f;
      const float masterVol = std::clamp(params_[kParamMasterVolume], 0.0f, 1.0f);

      sumL *= driveGain;
      sumR *= driveGain;
      sumL = std::tanh(sumL);
      sumR = std::tanh(sumR);
      sumL *= masterVol;
      sumR *= masterVol;

      left[frame] = sumL;
      right[frame] = sumR;
    }
  }

private:
  struct VoiceSlot {
    dsp::SynthVoice voice{};
    int8_t note = -1;
    float velocity = 0.0f;
    uint64_t age = 0;
    bool held = false;
  };

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

  void noteOn(uint8_t note, uint8_t velocity) {
    const float vel = std::clamp(velocity / 127.0f, 0.0f, 1.0f);

    if (polyMode_) {
      polyNoteOn(note, vel);
    } else {
      monoNoteOn(note, vel);
    }
  }

  void noteOff(uint8_t note) {
    if (polyMode_) {
      polyNoteOff(note);
    } else {
      monoNoteOff(note);
    }
  }

  // === Polyphonic mode ===

  void polyNoteOn(uint8_t note, float velocity) {
    // First check if note is already playing — retrigger on same voice
    for (auto& slot : voices_) {
      if (slot.note == static_cast<int8_t>(note) && slot.held) {
        slot.voice.noteOn(note);
        slot.velocity = velocity;
        slot.age = globalAge_;
        return;
      }
    }

    // Find a free voice
    for (auto& slot : voices_) {
      if (!slot.voice.isActive() && !slot.held) {
        activateSlot(slot, note, velocity);
        return;
      }
    }

    // Voice stealing: steal oldest voice
    VoiceSlot* oldest = &voices_[0];
    for (auto& slot : voices_) {
      if (slot.age < oldest->age)
        oldest = &slot;
    }
    activateSlot(*oldest, note, velocity);
  }

  void polyNoteOff(uint8_t note) {
    for (auto& slot : voices_) {
      if (slot.note == static_cast<int8_t>(note) && slot.held) {
        slot.held = false;
        slot.voice.noteOff();
      }
    }
  }

  void activateSlot(VoiceSlot& slot, uint8_t note, float velocity) {
    slot.note = static_cast<int8_t>(note);
    slot.velocity = velocity;
    slot.age = globalAge_;
    slot.held = true;

    // Re-apply wavetable pointer after potential voice reuse
    slot.voice.setWavetable(&wavetable_);
    slot.voice.noteOn(note);
  }

  // === Monophonic mode ===

  void monoNoteOn(uint8_t note, float velocity) {
    noteVelocity_[note] = velocity;
    removeHeldNote(note);
    pushHeldNote(note);

    auto& slot = voices_[0];
    slot.note = static_cast<int8_t>(note);
    slot.velocity = velocity;
    slot.age = globalAge_;
    slot.held = true;
    slot.voice.noteOn(note);
  }

  void monoNoteOff(uint8_t note) {
    const bool removed = removeHeldNote(note);
    noteVelocity_[note] = 0.0f;

    if (!removed)
      return;

    auto& slot = voices_[0];

    if (monoHeldCount_ == 0) {
      slot.held = false;
      slot.note = -1;
      slot.voice.noteOff();
      return;
    }

    // Fall back to previous held note
    const uint8_t fallbackNote = monoHeldNotes_[monoHeldCount_ - 1];
    slot.note = static_cast<int8_t>(fallbackNote);
    slot.velocity = noteVelocity_[fallbackNote];
    slot.held = true;
    slot.voice.noteOn(fallbackNote);
  }

  void pushHeldNote(uint8_t note) {
    if (monoHeldCount_ < monoHeldNotes_.size()) {
      monoHeldNotes_[monoHeldCount_++] = note;
      return;
    }
    for (size_t i = 1; i < monoHeldNotes_.size(); ++i)
      monoHeldNotes_[i - 1] = monoHeldNotes_[i];
    monoHeldNotes_[monoHeldNotes_.size() - 1] = note;
  }

  bool removeHeldNote(uint8_t note) {
    for (size_t i = 0; i < monoHeldCount_; ++i) {
      if (monoHeldNotes_[i] != note)
        continue;
      for (size_t j = i + 1; j < monoHeldCount_; ++j)
        monoHeldNotes_[j - 1] = monoHeldNotes_[j];
      --monoHeldCount_;
      return true;
    }
    return false;
  }

  void allNotesOff() {
    for (auto& slot : voices_) {
      slot.held = false;
      slot.note = -1;
      slot.velocity = 0.0f;
      slot.voice.noteOff();
    }
    monoHeldCount_ = 0;
    noteVelocity_.fill(0.0f);
  }

  // Shared state
  double sampleRate_ = 44100.0;
  std::array<float, kParameterCount> params_{};
  std::array<float, kNumOscillators> oscLevels_{0.6f, 0.4f, 0.35f};
  bool polyMode_ = true;
  uint64_t globalAge_ = 0;

  // Wavetable (shared across all voices)
  dsp::Wavetable wavetable_{};

  // Voice pool
  std::array<VoiceSlot, kMaxPolyVoices> voices_{};
  dsp::LFO lfo_{};

  // Mono mode state
  std::array<float, 128> noteVelocity_{};
  std::array<uint8_t, 16> monoHeldNotes_{};
  size_t monoHeldCount_ = 0;
};

} // namespace venom::core
