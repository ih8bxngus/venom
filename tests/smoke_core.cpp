#include <cmath>
#include <iostream>
#include <vector>

#include "../src/core/SynthEngine.hpp"

int main() {
  venom::core::SynthEngine engine;
  engine.setSampleRate(48000.0);

  std::array<float, venom::kParameterCount> params{};
  params[venom::kParamMasterVolume] = 0.8f;
  params[venom::kParamDrive] = 0.2f;
  params[venom::kParamOsc1Wave] = 1.0f;
  params[venom::kParamOsc1Detune] = -5.0f;
  params[venom::kParamOsc1Pitch] = 0.0f;
  params[venom::kParamOsc1Level] = 0.6f;
  params[venom::kParamOsc2Wave] = 2.0f;
  params[venom::kParamOsc2Detune] = 5.0f;
  params[venom::kParamOsc2Pitch] = 0.0f;
  params[venom::kParamOsc2Level] = 0.5f;
  params[venom::kParamOsc3Wave] = 0.0f;
  params[venom::kParamOsc3Detune] = 0.0f;
  params[venom::kParamOsc3Pitch] = -12.0f;
  params[venom::kParamOsc3Level] = 0.4f;
  params[venom::kParamFilterCutoff] = 1800.0f;
  params[venom::kParamFilterResonance] = 0.25f;
  params[venom::kParamFilterEnvAmount] = 0.3f;
  params[venom::kParamAmpAttack] = 0.01f;
  params[venom::kParamAmpDecay] = 0.15f;
  params[venom::kParamAmpSustain] = 0.7f;
  params[venom::kParamAmpRelease] = 0.2f;
  params[venom::kParamFilterAttack] = 0.01f;
  params[venom::kParamFilterDecay] = 0.1f;
  params[venom::kParamFilterSustain] = 0.5f;
  params[venom::kParamFilterRelease] = 0.2f;
  params[venom::kParamLfoRate] = 3.0f;
  params[venom::kParamLfoDepth] = 0.5f;
  params[venom::kParamLfoTarget] = 0.0f;
  params[venom::kParamEnvRetrigger] = 1.0f;

  engine.setParameters(params);

  constexpr uint32_t kFrames = 2048;
  std::vector<float> left(kFrames, 0.0f);
  std::vector<float> right(kFrames, 0.0f);

  venom::core::MidiEvent events[2] = {
      {.frame = 0, .status = 0x90, .data1 = 48, .data2 = 100},
      {.frame = 1200, .status = 0x80, .data1 = 48, .data2 = 0},
  };

  engine.process(events, 2, left.data(), right.data(), kFrames);

  float peak = 0.0f;
  for (float s : left) {
    if (!std::isfinite(s)) {
      std::cerr << "non-finite sample detected\n";
      return 1;
    }
    peak = std::max(peak, std::abs(s));
  }

  if (peak < 1.0e-4f) {
    std::cerr << "peak too low; synth did not produce expected signal\n";
    return 2;
  }

  std::cout << "core smoke test peak=" << peak << "\n";
  return 0;
}
