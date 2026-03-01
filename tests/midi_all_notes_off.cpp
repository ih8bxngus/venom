#include <cmath>
#include <iostream>
#include <vector>

#include "../src/core/SynthEngine.hpp"

int main() {
  venom::core::SynthEngine engine;
  engine.setSampleRate(48000.0);

  std::array<float, venom::kParameterCount> params{};
  params[venom::kParamMasterVolume] = 0.8f;
  params[venom::kParamDrive] = 0.05f;
  params[venom::kParamOsc1Wave] = 1.0f;
  params[venom::kParamOsc1Level] = 1.0f;
  params[venom::kParamFilterCutoff] = 18000.0f;
  params[venom::kParamFilterResonance] = 0.0f;
  params[venom::kParamAmpAttack] = 0.001f;
  params[venom::kParamAmpDecay] = 0.01f;
  params[venom::kParamAmpSustain] = 1.0f;
  params[venom::kParamAmpRelease] = 0.02f;
  params[venom::kParamFilterAttack] = 0.001f;
  params[venom::kParamFilterDecay] = 0.01f;
  params[venom::kParamFilterSustain] = 1.0f;
  params[venom::kParamFilterRelease] = 0.02f;
  params[venom::kParamLfoRate] = 2.0f;
  params[venom::kParamLfoDepth] = 0.0f;
  params[venom::kParamEnvRetrigger] = 1.0f;
  engine.setParameters(params);

  constexpr uint32_t kFrames = 4096;
  std::vector<float> left(kFrames, 0.0f);
  std::vector<float> right(kFrames, 0.0f);

  venom::core::MidiEvent events[2] = {
      {.frame = 0, .status = 0x90, .data1 = 52, .data2 = 105},
      {.frame = 700, .status = 0xB0, .data1 = 123, .data2 = 0},
  };

  engine.process(events, 2, left.data(), right.data(), kFrames);

  float preAllOffEnergy = 0.0f;
  for (uint32_t i = 100; i < 600; ++i)
    preAllOffEnergy += std::abs(left[i]);

  float tailEnergy = 0.0f;
  for (uint32_t i = 3300; i < 3900; ++i)
    tailEnergy += std::abs(left[i]);

  if (preAllOffEnergy <= 1.0e-3f) {
    std::cerr << "no energy before all-notes-off\n";
    return 1;
  }

  if (tailEnergy >= preAllOffEnergy * 0.12f) {
    std::cerr << "all-notes-off did not silence tail enough\n";
    return 2;
  }

  std::cout << "midi all-notes-off test passed\n";
  return 0;
}
