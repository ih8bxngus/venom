#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include "../src/core/SynthEngine.hpp"

static std::array<float, venom::kParameterCount> makePolyParams() {
  std::array<float, venom::kParameterCount> params{};
  params[venom::kParamMasterVolume] = 0.8f;
  params[venom::kParamDrive] = 0.1f;
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
  params[venom::kParamAmpRelease] = 0.05f;
  params[venom::kParamFilterAttack] = 0.001f;
  params[venom::kParamFilterDecay] = 0.01f;
  params[venom::kParamFilterSustain] = 1.0f;
  params[venom::kParamFilterRelease] = 0.05f;
  params[venom::kParamLfoDepth] = 0.0f;
  params[venom::kParamEnvRetrigger] = 1.0f;
  params[venom::kParamPolyMode] = 1.0f;       // Poly ON
  params[venom::kParamUnisonCount] = 1.0f;     // No unison
  params[venom::kParamUnisonDetune] = 15.0f;
  params[venom::kParamUnisonSpread] = 0.5f;
  return params;
}

int main() {
  // Use heap to avoid stack overflow (SynthEngine is large due to Wavetable + 8 voices)
  auto engine = std::make_unique<venom::core::SynthEngine>();
  engine->setSampleRate(48000.0);
  engine->setParameters(makePolyParams());

  constexpr uint32_t kFrames = 4096;
  std::vector<float> left(kFrames, 0.0f);
  std::vector<float> right(kFrames, 0.0f);

  // Play single note first
  {
    venom::core::MidiEvent events[2] = {
        {.frame = 0, .status = 0x90, .data1 = 60, .data2 = 100},
        {.frame = 2048, .status = 0x80, .data1 = 60, .data2 = 0},
    };
    engine->process(events, 2, left.data(), right.data(), kFrames);
  }

  float singlePeak = 0.0f;
  for (uint32_t i = 200; i < 1800; ++i)
    singlePeak = std::max(singlePeak, std::abs(left[i]));

  if (singlePeak < 1.0e-3f) {
    std::cerr << "single note produced no signal\n";
    return 1;
  }

  // Reset and play two simultaneous notes using a fresh engine
  std::fill(left.begin(), left.end(), 0.0f);
  std::fill(right.begin(), right.end(), 0.0f);

  // Reuse same engine pointer (destroy old, create new)
  engine = std::make_unique<venom::core::SynthEngine>();
  engine->setSampleRate(48000.0);
  engine->setParameters(makePolyParams());

  {
    venom::core::MidiEvent events[4] = {
        {.frame = 0, .status = 0x90, .data1 = 60, .data2 = 100},
        {.frame = 0, .status = 0x90, .data1 = 67, .data2 = 100},
        {.frame = 2048, .status = 0x80, .data1 = 60, .data2 = 0},
        {.frame = 2048, .status = 0x80, .data1 = 67, .data2 = 0},
    };
    engine->process(events, 4, left.data(), right.data(), kFrames);
  }

  float dualPeak = 0.0f;
  for (uint32_t i = 200; i < 1800; ++i)
    dualPeak = std::max(dualPeak, std::abs(left[i]));

  // With tanh saturation, two voices compress — just check dual is measurably louder
  if (dualPeak < singlePeak * 1.05f) {
    std::cerr << "two notes didn't produce louder signal than one (single="
              << singlePeak << " dual=" << dualPeak << ")\n";
    return 2;
  }

  std::cout << "polyphony basic test passed (single=" << singlePeak
            << " dual=" << dualPeak << ")\n";
  return 0;
}
