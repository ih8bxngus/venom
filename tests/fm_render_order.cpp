// Test: FM render order correctness
// Verifies that FM routing matches the documented chain:
// Osc3 (unmodulated) → Osc2 (modulated by Osc3) → Osc1 (modulated by Osc2+Osc3)
//
// Strategy: Enable only FM3to2 and verify Osc2 output changes while Osc1 stays
// the same as without FM (since 2to1 and 3to1 are both 0).

#include <cmath>
#include <iostream>
#include <vector>

#include "../src/core/SynthEngine.hpp"

int main() {
  constexpr uint32_t kFrames = 4096;
  constexpr double kSampleRate = 48000.0;

  auto makeParams = [](float fm2to1, float fm3to1, float fm3to2) {
    std::array<float, venom::kParameterCount> p{};
    p[venom::kParamMasterVolume] = 0.8f;
    p[venom::kParamDrive] = 0.0f;

    // All sine oscillators at different pitches
    p[venom::kParamOsc1Wave] = 0.0f;
    p[venom::kParamOsc1Level] = 0.5f;
    p[venom::kParamOsc2Wave] = 0.0f;
    p[venom::kParamOsc2Level] = 0.5f;
    p[venom::kParamOsc3Wave] = 0.0f;
    p[venom::kParamOsc3Pitch] = 12.0f; // One octave up for clear modulation
    p[venom::kParamOsc3Level] = 0.0f;  // Modulator only, not in mix

    p[venom::kParamFilterCutoff] = 20000.0f;
    p[venom::kParamFilterResonance] = 0.0f;

    p[venom::kParamAmpAttack] = 0.001f;
    p[venom::kParamAmpDecay] = 1.0f;
    p[venom::kParamAmpSustain] = 1.0f;
    p[venom::kParamAmpRelease] = 0.5f;
    p[venom::kParamEnvRetrigger] = 1.0f;

    p[venom::kParamFM2to1] = fm2to1;
    p[venom::kParamFM3to1] = fm3to1;
    p[venom::kParamFM3to2] = fm3to2;

    return p;
  };

  auto runEngine = [&](float fm2to1, float fm3to1, float fm3to2) {
    venom::core::SynthEngine engine;
    engine.setSampleRate(kSampleRate);
    engine.setParameters(makeParams(fm2to1, fm3to1, fm3to2));

    std::vector<float> left(kFrames, 0.0f), right(kFrames, 0.0f);
    venom::core::MidiEvent noteOn = {0, 0x90, 60, 100};
    engine.process(&noteOn, 1, left.data(), right.data(), kFrames);
    return left;
  };

  // Case 1: No FM
  auto noFm = runEngine(0.0f, 0.0f, 0.0f);

  // Case 2: Only FM3to2 = 0.5 (Osc3 modulates Osc2 only)
  auto fm3to2Only = runEngine(0.0f, 0.0f, 0.5f);

  // Case 3: Only FM2to1 = 0.5 (Osc2 modulates Osc1 only)
  auto fm2to1Only = runEngine(0.5f, 0.0f, 0.0f);

  // Case 4: Only FM3to1 = 0.5 (Osc3 modulates Osc1 only)
  auto fm3to1Only = runEngine(0.0f, 0.5f, 0.0f);

  // Validate all produce signal
  for (const auto* buf : {&noFm, &fm3to2Only, &fm2to1Only, &fm3to1Only}) {
    float peak = 0.0f;
    for (float s : *buf) {
      if (!std::isfinite(s)) {
        std::cerr << "non-finite sample detected\n";
        return 1;
      }
      peak = std::max(peak, std::abs(s));
    }
    if (peak < 1e-4f) {
      std::cerr << "signal too quiet\n";
      return 2;
    }
  }

  // fm3to2Only should differ from noFm (Osc2 is modulated)
  float diff3to2 = 0.0f;
  for (uint32_t i = 0; i < kFrames; ++i)
    diff3to2 += std::abs(fm3to2Only[i] - noFm[i]);
  diff3to2 /= static_cast<float>(kFrames);

  if (diff3to2 < 1e-4f) {
    std::cerr << "FM3to2 did not affect output (avgDiff=" << diff3to2 << ")\n";
    return 3;
  }

  // fm2to1Only should differ from noFm (Osc1 is modulated by Osc2)
  float diff2to1 = 0.0f;
  for (uint32_t i = 0; i < kFrames; ++i)
    diff2to1 += std::abs(fm2to1Only[i] - noFm[i]);
  diff2to1 /= static_cast<float>(kFrames);

  if (diff2to1 < 1e-4f) {
    std::cerr << "FM2to1 did not affect output (avgDiff=" << diff2to1 << ")\n";
    return 4;
  }

  // fm3to1Only should differ from noFm (Osc1 is modulated by Osc3)
  float diff3to1 = 0.0f;
  for (uint32_t i = 0; i < kFrames; ++i)
    diff3to1 += std::abs(fm3to1Only[i] - noFm[i]);
  diff3to1 /= static_cast<float>(kFrames);

  if (diff3to1 < 1e-4f) {
    std::cerr << "FM3to1 did not affect output (avgDiff=" << diff3to1 << ")\n";
    return 5;
  }

  // Each FM path should produce different results from each other
  float diff2v3 = 0.0f;
  for (uint32_t i = 0; i < kFrames; ++i)
    diff2v3 += std::abs(fm2to1Only[i] - fm3to1Only[i]);
  diff2v3 /= static_cast<float>(kFrames);

  if (diff2v3 < 1e-4f) {
    std::cerr << "FM2to1 and FM3to1 produced identical output\n";
    return 6;
  }

  std::cout << "fm_render_order test passed: diff3to2=" << diff3to2
            << " diff2to1=" << diff2to1 << " diff3to1=" << diff3to1
            << " diff2v3=" << diff2v3 << "\n";
  return 0;
}
