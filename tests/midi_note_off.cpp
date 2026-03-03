#include <cmath>
#include <iostream>
#include <vector>

#include "../src/core/SynthEngine.hpp"

int main() {
  venom::core::SynthEngine engine;
  engine.setSampleRate(48000.0);

  std::array<float, venom::kParameterCount> params{};
  params[venom::kParamMasterVolume] = 0.8f;
  params[venom::kParamDrive] = 0.1f;
  params[venom::kParamOsc1Wave] = 1.0f;
  params[venom::kParamOsc1Level] = 1.0f;
  params[venom::kParamOsc2Wave] = 0.0f;
  params[venom::kParamOsc2Level] = 0.0f;
  params[venom::kParamOsc3Wave] = 0.0f;
  params[venom::kParamOsc3Level] = 0.0f;
  params[venom::kParamFilterCutoff] = 20000.0f;
  params[venom::kParamFilterResonance] = 0.0f;
  params[venom::kParamFilterEnvAmount] = 0.0f;
  params[venom::kParamAmpAttack] = 0.001f;
  params[venom::kParamAmpDecay] = 0.01f;
  params[venom::kParamAmpSustain] = 1.0f;
  params[venom::kParamAmpRelease] = 0.03f;
  params[venom::kParamFilterAttack] = 0.001f;
  params[venom::kParamFilterDecay] = 0.01f;
  params[venom::kParamFilterSustain] = 1.0f;
  params[venom::kParamFilterRelease] = 0.03f;
  params[venom::kParamLfoRate] = 3.0f;
  params[venom::kParamLfoDepth] = 0.0f;
  params[venom::kParamLfoTarget] = 0.0f;
  params[venom::kParamEnvRetrigger] = 1.0f;
  engine.setParameters(params);

  constexpr uint32_t kFrames = 4096;
  std::vector<float> left(kFrames, 0.0f);
  std::vector<float> right(kFrames, 0.0f);

  venom::core::MidiEvent events[2] = {
      {.frame = 0, .status = 0x90, .data1 = 60, .data2 = 110},
      {.frame = 512, .status = 0x80, .data1 = 60, .data2 = 0},
  };

  engine.process(events, 2, left.data(), right.data(), kFrames);

  float preOffEnergy = 0.0f;
  for (uint32_t i = 100; i < 500; ++i)
    preOffEnergy += std::abs(left[i]);

  float tailEnergy = 0.0f;
  for (uint32_t i = 3500; i < 4000; ++i)
    tailEnergy += std::abs(left[i]);

  if (preOffEnergy <= 1.0e-3f) {
    std::cerr << "no pre-noteoff energy\n";
    return 1;
  }

  if (tailEnergy >= preOffEnergy * 0.3f) {
    std::cerr << "tail energy remained too high after note-off\n";
    return 2;
  }

  std::cout << "midi note-off test passed\n";
  return 0;
}
