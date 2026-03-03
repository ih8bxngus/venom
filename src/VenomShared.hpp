#pragma once

#include <array>
#include <cstdint>

namespace venom {

enum class Waveform : uint8_t {
  Sine = 0,
  Saw,
  Square,
  Triangle,
  Wavetable,
};

constexpr uint32_t kNumWaveforms = 5;
constexpr uint32_t kNumOscillators = 3;
constexpr uint32_t kNumEnvelopes = 2;
constexpr uint32_t kMaxPolyVoices = 8;
constexpr uint32_t kMaxUnisonVoices = 7;

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

  // === v0.4 new parameters (appended for backward compat) ===
  kParamPolyMode,       // 0=Mono, 1=Poly
  kParamUnisonCount,    // 1-7
  kParamUnisonDetune,   // 0-100 cents
  kParamUnisonSpread,   // 0-1 stereo width
  kParamOsc1WavePos,    // 0-1 wavetable position
  kParamOsc2WavePos,    // 0-1
  kParamOsc3WavePos,    // 0-1
  kParamLfoWave,        // 0-4 (Sine/Saw/Sq/Tri/S&H)

  // === v0.5 Phase 1: FM Synthesis ===
  kParamFM2to1,          // 0-1, FM amount Osc2 -> Osc1
  kParamFM3to1,          // 0-1, FM amount Osc3 -> Osc1
  kParamFM3to2,          // 0-1, FM amount Osc3 -> Osc2

  // === v0.5 Phase 2: Wavetable Banks ===
  kParamOsc1WtBank,      // 0-9, wavetable bank index
  kParamOsc2WtBank,      // 0-9
  kParamOsc3WtBank,      // 0-9

  // === v0.5 Phase 3: Sample/Granular ===
  kParamOsc1Engine,      // 0=Classic, 1=Wavetable, 2=Granular
  kParamOsc2Engine,      // 0=Classic, 1=Wavetable, 2=Granular
  kParamOsc3Engine,      // 0=Classic, 1=Wavetable, 2=Granular
  kParamGrainSize,       // 5-500 ms
  kParamGrainDensity,    // 1-64 grains/sec
  kParamGrainScatter,    // 0-1
  kParamGrainPitchRand,  // 0-1
  kParamSampleStart,     // 0-1
  kParamSampleSelect,    // 0-4 (built-in sample index)
  kParamGrainWindow,     // 0=Hann, 1=Tukey, 2=Gaussian

  kParameterCount        // = 53
};

// For backward-compatible preset loading
static constexpr uint32_t kOldParameterCount_v1 = 29;
static constexpr uint32_t kOldParameterCount_v2 = 37;

// Oscillator engine types
enum class OscEngine : uint8_t {
  Classic = 0,    // Sine/Saw/Square/Triangle with polyBLEP
  Wavetable = 1,  // Wavetable bank playback
  Granular = 2,   // Sample/Granular playback
};

constexpr uint32_t kNumWavetableBanks = 10;
constexpr uint32_t kNumBuiltInSamples = 5;

} // namespace venom
