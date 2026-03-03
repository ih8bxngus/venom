#include <cmath>
#include <iostream>
#include <vector>

#include "../src/core/SynthEngine.hpp"

static std::array<float, venom::kParameterCount> makeBaseParams() {
  std::array<float, venom::kParameterCount> params{};
  params[venom::kParamMasterVolume] = 0.9f;
  params[venom::kParamDrive] = 0.0f;
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
  params[venom::kParamAmpRelease] = 0.005f;
  params[venom::kParamFilterAttack] = 0.001f;
  params[venom::kParamFilterDecay] = 0.01f;
  params[venom::kParamFilterSustain] = 1.0f;
  params[venom::kParamFilterRelease] = 0.005f;
  params[venom::kParamLfoRate] = 2.0f;
  params[venom::kParamLfoDepth] = 0.0f;
  params[venom::kParamLfoTarget] = 0.0f;
  params[venom::kParamEnvRetrigger] = 1.0f;
  return params;
}

int main() {
  venom::core::SynthEngine engine;
  engine.setSampleRate(48000.0);
  engine.setParameters(makeBaseParams());

  constexpr uint32_t kFrames = 2048;
  std::vector<float> left(kFrames, 0.0f);
  std::vector<float> right(kFrames, 0.0f);

  venom::core::MidiEvent events[4] = {
      {.frame = 0, .status = 0x90, .data1 = 60, .data2 = 95},
      {.frame = 240, .status = 0x90, .data1 = 72, .data2 = 120},
      {.frame = 480, .status = 0x80, .data1 = 72, .data2 = 0},
      {.frame = 960, .status = 0x80, .data1 = 60, .data2 = 0},
  };

  engine.process(events, 4, left.data(), right.data(), kFrames);

  float afterTopNoteOffEnergy = 0.0f;
  for (uint32_t i = 560; i < 840; ++i)
    afterTopNoteOffEnergy += std::abs(left[i]);

  float afterAllNotesOffEnergy = 0.0f;
  for (uint32_t i = 1500; i < 1850; ++i)
    afterAllNotesOffEnergy += std::abs(left[i]);

  if (afterTopNoteOffEnergy <= 0.2f) {
    std::cerr << "engine did not fall back to previously held note\n";
    return 1;
  }

  if (afterAllNotesOffEnergy >= afterTopNoteOffEnergy * 0.2f) {
    std::cerr << "engine energy stayed too high after all notes released\n";
    return 2;
  }

  std::cout << "midi last-note-priority test passed\n";
  return 0;
}
