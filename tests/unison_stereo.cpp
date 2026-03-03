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
  params[venom::kParamOsc1Wave] = 1.0f; // Saw
  params[venom::kParamOsc1Level] = 0.8f;
  params[venom::kParamOsc2Level] = 0.0f;
  params[venom::kParamOsc3Level] = 0.0f;
  params[venom::kParamFilterCutoff] = 20000.0f;
  params[venom::kParamFilterResonance] = 0.0f;
  params[venom::kParamFilterEnvAmount] = 0.0f;
  params[venom::kParamAmpAttack] = 0.001f;
  params[venom::kParamAmpDecay] = 0.01f;
  params[venom::kParamAmpSustain] = 1.0f;
  params[venom::kParamAmpRelease] = 0.1f;
  params[venom::kParamFilterAttack] = 0.001f;
  params[venom::kParamFilterDecay] = 0.01f;
  params[venom::kParamFilterSustain] = 1.0f;
  params[venom::kParamFilterRelease] = 0.1f;
  params[venom::kParamLfoDepth] = 0.0f;
  params[venom::kParamEnvRetrigger] = 1.0f;
  params[venom::kParamPolyMode] = 1.0f;
  params[venom::kParamUnisonCount] = 5.0f;     // 5 unison voices
  params[venom::kParamUnisonDetune] = 30.0f;    // 30 cents spread
  params[venom::kParamUnisonSpread] = 1.0f;     // Full stereo spread
  engine.setParameters(params);

  constexpr uint32_t kFrames = 4096;
  std::vector<float> left(kFrames, 0.0f);
  std::vector<float> right(kFrames, 0.0f);

  venom::core::MidiEvent events[2] = {
      {.frame = 0, .status = 0x90, .data1 = 60, .data2 = 100},
      {.frame = 3000, .status = 0x80, .data1 = 60, .data2 = 0},
  };

  engine.process(events, 2, left.data(), right.data(), kFrames);

  // Check that both channels have signal
  float leftEnergy = 0.0f;
  float rightEnergy = 0.0f;
  for (uint32_t i = 400; i < 2500; ++i) {
    leftEnergy += std::abs(left[i]);
    rightEnergy += std::abs(right[i]);
  }

  if (leftEnergy < 1.0e-2f || rightEnergy < 1.0e-2f) {
    std::cerr << "no signal in one or both channels (L=" << leftEnergy
              << " R=" << rightEnergy << ")\n";
    return 1;
  }

  // With unison spread=1.0, L and R should differ
  float diffEnergy = 0.0f;
  for (uint32_t i = 400; i < 2500; ++i)
    diffEnergy += std::abs(left[i] - right[i]);

  if (diffEnergy < 1.0e-3f) {
    std::cerr << "L and R are identical despite unison spread — no stereo effect\n";
    return 2;
  }

  std::cout << "unison stereo test passed (L=" << leftEnergy
            << " R=" << rightEnergy << " diff=" << diffEnergy << ")\n";
  return 0;
}
