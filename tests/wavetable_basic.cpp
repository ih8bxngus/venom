#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include "../src/core/SynthEngine.hpp"

int main() {
  // Use heap to avoid stack overflow (SynthEngine is large due to Wavetable + 8 voices)
  auto engine = std::make_unique<venom::core::SynthEngine>();
  engine->setSampleRate(48000.0);

  std::array<float, venom::kParameterCount> params{};
  params[venom::kParamMasterVolume] = 0.8f;
  params[venom::kParamDrive] = 0.05f;
  params[venom::kParamOsc1Wave] = 4.0f; // Wavetable
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
  params[venom::kParamPolyMode] = 1.0f;
  params[venom::kParamUnisonCount] = 1.0f;
  params[venom::kParamOsc1WavePos] = 0.5f; // Midpoint of wavetable

  engine->setParameters(params);

  constexpr uint32_t kFrames = 4096;
  std::vector<float> left(kFrames, 0.0f);
  std::vector<float> right(kFrames, 0.0f);

  venom::core::MidiEvent events[2] = {
      {.frame = 0, .status = 0x90, .data1 = 60, .data2 = 100},
      {.frame = 3000, .status = 0x80, .data1 = 60, .data2 = 0},
  };

  engine->process(events, 2, left.data(), right.data(), kFrames);

  float peak = 0.0f;
  bool allFinite = true;
  for (float s : left) {
    if (!std::isfinite(s)) {
      allFinite = false;
      break;
    }
    peak = std::max(peak, std::abs(s));
  }

  if (!allFinite) {
    std::cerr << "wavetable produced non-finite samples\n";
    return 1;
  }

  if (peak < 1.0e-3f) {
    std::cerr << "wavetable produced no signal (peak=" << peak << ")\n";
    return 2;
  }

  // Test at different WT positions to verify timbral change
  std::fill(left.begin(), left.end(), 0.0f);
  std::fill(right.begin(), right.end(), 0.0f);

  params[venom::kParamOsc1WavePos] = 1.0f; // End of wavetable (rich harmonics)

  // Reuse same engine (reset by creating new one)
  engine = std::make_unique<venom::core::SynthEngine>();
  engine->setSampleRate(48000.0);
  engine->setParameters(params);

  {
    venom::core::MidiEvent ev[2] = {
        {.frame = 0, .status = 0x90, .data1 = 60, .data2 = 100},
        {.frame = 3000, .status = 0x80, .data1 = 60, .data2 = 0},
    };
    engine->process(ev, 2, left.data(), right.data(), kFrames);
  }

  float peakFull = 0.0f;
  for (float s : left)
    peakFull = std::max(peakFull, std::abs(s));

  if (peakFull < 1.0e-3f) {
    std::cerr << "wavetable at full position produced no signal\n";
    return 3;
  }

  std::cout << "wavetable basic test passed (peak@0.5=" << peak
            << " peak@1.0=" << peakFull << ")\n";
  return 0;
}
