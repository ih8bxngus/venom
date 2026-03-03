// Test: FM synthesis produces audible modulation
// Sets FM3to1 = 0.5 with sine waves and verifies the output differs from
// an unmodulated sine (more harmonic content means higher peak).

#include <cmath>
#include <iostream>
#include <vector>

#include "../src/core/SynthEngine.hpp"

int main() {
  constexpr uint32_t kFrames = 4096;
  constexpr double kSampleRate = 48000.0;

  // Helper to configure a basic sine patch
  auto makeParams = [](float fm3to1) {
    std::array<float, venom::kParameterCount> p{};
    p[venom::kParamMasterVolume] = 0.8f;
    p[venom::kParamDrive] = 0.0f; // No drive for clean test

    // All oscillators sine
    p[venom::kParamOsc1Wave] = 0.0f; // Sine
    p[venom::kParamOsc1Level] = 0.7f;
    p[venom::kParamOsc2Wave] = 0.0f;
    p[venom::kParamOsc2Level] = 0.0f; // Silent in mix
    p[venom::kParamOsc3Wave] = 0.0f;
    p[venom::kParamOsc3Pitch] = 7.0f; // Different ratio for interesting modulation
    p[venom::kParamOsc3Level] = 0.0f; // Silent in mix (modulator only)

    p[venom::kParamFilterCutoff] = 20000.0f; // Wide open
    p[venom::kParamFilterResonance] = 0.0f;

    p[venom::kParamAmpAttack] = 0.001f;
    p[venom::kParamAmpDecay] = 0.5f;
    p[venom::kParamAmpSustain] = 1.0f;
    p[venom::kParamAmpRelease] = 0.5f;
    p[venom::kParamEnvRetrigger] = 1.0f;

    // FM routing
    p[venom::kParamFM3to1] = fm3to1;

    return p;
  };

  // --- Run with FM OFF ---
  venom::core::SynthEngine engineOff;
  engineOff.setSampleRate(kSampleRate);
  engineOff.setParameters(makeParams(0.0f));

  std::vector<float> leftOff(kFrames, 0.0f), rightOff(kFrames, 0.0f);
  venom::core::MidiEvent noteOn = {0, 0x90, 60, 100};
  engineOff.process(&noteOn, 1, leftOff.data(), rightOff.data(), kFrames);

  // --- Run with FM ON ---
  venom::core::SynthEngine engineOn;
  engineOn.setSampleRate(kSampleRate);
  engineOn.setParameters(makeParams(0.5f));

  std::vector<float> leftOn(kFrames, 0.0f), rightOn(kFrames, 0.0f);
  engineOn.process(&noteOn, 1, leftOn.data(), rightOn.data(), kFrames);

  // --- Validate ---
  // Both should produce signal
  float peakOff = 0.0f, peakOn = 0.0f;
  for (uint32_t i = 0; i < kFrames; ++i) {
    if (!std::isfinite(leftOn[i]) || !std::isfinite(leftOff[i])) {
      std::cerr << "non-finite sample at frame " << i << "\n";
      return 1;
    }
    peakOff = std::max(peakOff, std::abs(leftOff[i]));
    peakOn = std::max(peakOn, std::abs(leftOn[i]));
  }

  if (peakOff < 1e-4f) {
    std::cerr << "FM-off signal too quiet: " << peakOff << "\n";
    return 2;
  }
  if (peakOn < 1e-4f) {
    std::cerr << "FM-on signal too quiet: " << peakOn << "\n";
    return 3;
  }

  // The modulated signal should differ from unmodulated
  float diffSum = 0.0f;
  for (uint32_t i = 0; i < kFrames; ++i)
    diffSum += std::abs(leftOn[i] - leftOff[i]);

  float avgDiff = diffSum / static_cast<float>(kFrames);
  if (avgDiff < 1e-4f) {
    std::cerr << "FM modulation produced no audible difference (avg diff=" << avgDiff << ")\n";
    return 4;
  }

  std::cout << "fm_basic test passed: peakOff=" << peakOff
            << " peakOn=" << peakOn << " avgDiff=" << avgDiff << "\n";
  return 0;
}
