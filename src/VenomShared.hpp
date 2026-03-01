#pragma once

#include <array>
#include <cstdint>

namespace venom {

enum class Waveform : uint8_t {
  Sine = 0,
  Saw,
  Square,
  Triangle,
};

constexpr uint32_t kNumOscillators = 3;
constexpr uint32_t kNumEnvelopes = 2;

enum ParameterIndex : uint32_t {
  kParamMasterVolume = 0,
  kParamDrive,

  kParamOsc1Wave,
  kParamOsc1Detune,
  kParamOsc1Pitch,
  kParamOsc1Level,

  kParamOsc2Wave,
  kParamOsc2Detune,
  kParamOsc2Pitch,
  kParamOsc2Level,

  kParamOsc3Wave,
  kParamOsc3Detune,
  kParamOsc3Pitch,
  kParamOsc3Level,

  kParamFilterCutoff,
  kParamFilterResonance,
  kParamFilterEnvAmount,

  kParamAmpAttack,
  kParamAmpDecay,
  kParamAmpSustain,
  kParamAmpRelease,

  kParamFilterAttack,
  kParamFilterDecay,
  kParamFilterSustain,
  kParamFilterRelease,

  kParamLfoRate,
  kParamLfoDepth,
  kParamLfoTarget,

  kParamEnvRetrigger,
  kParameterCount
};

} // namespace venom
