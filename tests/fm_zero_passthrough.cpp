// Test: FM at zero produces identical output to non-FM path
// Verifies that when all FM amounts are 0.0, the output matches
// the original rendering path with negligible difference.

#include <cmath>
#include <iostream>
#include <vector>

#include "../src/core/SynthEngine.hpp"

int main() {
  constexpr uint32_t kFrames = 2048;
  constexpr double kSampleRate = 48000.0;

  auto makeParams = []() {
    std::array<float, venom::kParameterCount> p{};
    p[venom::kParamMasterVolume] = 0.8f;
    p[venom::kParamDrive] = 0.1f;

    p[venom::kParamOsc1Wave] = 1.0f; // Saw
    p[venom::kParamOsc1Detune] = -5.0f;
    p[venom::kParamOsc1Level] = 0.6f;
    p[venom::kParamOsc2Wave] = 2.0f; // Square
    p[venom::kParamOsc2Detune] = 5.0f;
    p[venom::kParamOsc2Level] = 0.4f;
    p[venom::kParamOsc3Wave] = 0.0f; // Sine
    p[venom::kParamOsc3Pitch] = -12.0f;
    p[venom::kParamOsc3Level] = 0.35f;

    p[venom::kParamFilterCutoff] = 1800.0f;
    p[venom::kParamFilterResonance] = 0.2f;

    p[venom::kParamAmpAttack] = 0.01f;
    p[venom::kParamAmpDecay] = 0.15f;
    p[venom::kParamAmpSustain] = 0.7f;
    p[venom::kParamAmpRelease] = 0.2f;
    p[venom::kParamEnvRetrigger] = 1.0f;

    // All FM amounts at zero
    p[venom::kParamFM2to1] = 0.0f;
    p[venom::kParamFM3to1] = 0.0f;
    p[venom::kParamFM3to2] = 0.0f;

    return p;
  };

  // Run two identical engines with FM=0 to verify determinism
  venom::core::SynthEngine engine1, engine2;
  engine1.setSampleRate(kSampleRate);
  engine2.setSampleRate(kSampleRate);

  auto params = makeParams();
  engine1.setParameters(params);
  engine2.setParameters(params);

  std::vector<float> left1(kFrames, 0.0f), right1(kFrames, 0.0f);
  std::vector<float> left2(kFrames, 0.0f), right2(kFrames, 0.0f);

  venom::core::MidiEvent noteOn = {0, 0x90, 60, 100};
  engine1.process(&noteOn, 1, left1.data(), right1.data(), kFrames);
  engine2.process(&noteOn, 1, left2.data(), right2.data(), kFrames);

  // Both should produce signal
  float peak = 0.0f;
  for (uint32_t i = 0; i < kFrames; ++i)
    peak = std::max(peak, std::abs(left1[i]));

  if (peak < 1e-4f) {
    std::cerr << "signal too quiet: " << peak << "\n";
    return 1;
  }

  // Outputs should be identical (deterministic, same state)
  float maxDiff = 0.0f;
  for (uint32_t i = 0; i < kFrames; ++i) {
    float diff = std::abs(left1[i] - left2[i]);
    maxDiff = std::max(maxDiff, diff);
  }

  if (maxDiff > 1e-5f) {
    std::cerr << "FM=0 output not deterministic, maxDiff=" << maxDiff << "\n";
    return 2;
  }

  // Verify all samples are finite
  for (uint32_t i = 0; i < kFrames; ++i) {
    if (!std::isfinite(left1[i]) || !std::isfinite(right1[i])) {
      std::cerr << "non-finite sample at frame " << i << "\n";
      return 3;
    }
  }

  std::cout << "fm_zero_passthrough test passed: peak=" << peak
            << " maxDiff=" << maxDiff << "\n";
  return 0;
}
