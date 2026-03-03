#include <cmath>
#include <iostream>
#include <string>

#include "../src/preset/PresetManager.hpp"

int main() {
  // Simulate an old v1 preset with only 29 params
  const std::string oldJson = R"({
  "version": 1,
  "name": "Legacy Test Preset",
  "params": [
    0.8, 0.15,
    1, -5, 0, 0.6,
    2, 5, 0, 0.4,
    0, 0, -12, 0.35,
    1200, 0.15, 0,
    0.01, 0.08, 0.85, 0.12,
    0.01, 0.12, 0.5, 0.15,
    3, 0.2, 0,
    1
  ]
})";

  venom::preset::Preset loaded;
  if (!venom::preset::PresetManager::fromJson(oldJson, loaded)) {
    std::cerr << "failed to parse legacy 29-param preset\n";
    return 1;
  }

  if (loaded.name != "Legacy Test Preset") {
    std::cerr << "preset name mismatch: " << loaded.name << "\n";
    return 2;
  }

  // Check original params preserved
  if (std::abs(loaded.params[venom::kParamMasterVolume] - 0.8f) > 1e-4f) {
    std::cerr << "master volume not preserved\n";
    return 3;
  }

  if (std::abs(loaded.params[venom::kParamOsc1Wave] - 1.0f) > 1e-4f) {
    std::cerr << "osc1 wave not preserved\n";
    return 4;
  }

  if (std::abs(loaded.params[venom::kParamEnvRetrigger] - 1.0f) > 1e-4f) {
    std::cerr << "env retrigger not preserved\n";
    return 5;
  }

  // Check v0.4 defaults were applied
  if (std::abs(loaded.params[venom::kParamPolyMode] - 1.0f) > 1e-4f) {
    std::cerr << "poly mode default incorrect: " << loaded.params[venom::kParamPolyMode] << "\n";
    return 6;
  }

  if (std::abs(loaded.params[venom::kParamUnisonCount] - 1.0f) > 1e-4f) {
    std::cerr << "unison count default incorrect\n";
    return 7;
  }

  if (std::abs(loaded.params[venom::kParamUnisonDetune] - 15.0f) > 1e-4f) {
    std::cerr << "unison detune default incorrect\n";
    return 8;
  }

  if (std::abs(loaded.params[venom::kParamUnisonSpread] - 0.5f) > 1e-4f) {
    std::cerr << "unison spread default incorrect\n";
    return 9;
  }

  if (std::abs(loaded.params[venom::kParamOsc1WavePos] - 0.0f) > 1e-4f) {
    std::cerr << "osc1 wave pos default incorrect\n";
    return 10;
  }

  if (std::abs(loaded.params[venom::kParamLfoWave] - 0.0f) > 1e-4f) {
    std::cerr << "lfo wave default incorrect\n";
    return 11;
  }

  // Check v0.5 defaults applied to v1 preset
  if (std::abs(loaded.params[venom::kParamFM2to1] - 0.0f) > 1e-4f) {
    std::cerr << "v1→v3: FM2to1 default incorrect\n";
    return 12;
  }
  if (std::abs(loaded.params[venom::kParamGrainSize] - 80.0f) > 1e-4f) {
    std::cerr << "v1→v3: GrainSize default incorrect\n";
    return 13;
  }

  // Verify v2 (37-param) presets migrate to v3 correctly
  const std::string v2Json = R"({
  "version": 2,
  "name": "V2 Format",
  "params": [
    0.7, 0.2,
    1, -3, 0, 0.5,
    2, 3, 0, 0.3,
    0, 0, -12, 0.25,
    1500, 0.2, 0.1,
    0.02, 0.1, 0.8, 0.15,
    0.02, 0.15, 0.6, 0.2,
    4, 0.3, 1,
    1,
    1, 3, 20, 0.7,
    0.3, 0.5, 0.1,
    2
  ]
})";

  venom::preset::Preset v2Loaded;
  if (!venom::preset::PresetManager::fromJson(v2Json, v2Loaded)) {
    std::cerr << "failed to parse 37-param v2 preset\n";
    return 14;
  }

  // Original v2 params preserved
  if (std::abs(v2Loaded.params[venom::kParamUnisonCount] - 3.0f) > 1e-4f) {
    std::cerr << "v2→v3: unison count not preserved\n";
    return 15;
  }

  if (std::abs(v2Loaded.params[venom::kParamLfoWave] - 2.0f) > 1e-4f) {
    std::cerr << "v2→v3: lfo wave not preserved\n";
    return 16;
  }

  // v0.5 defaults applied
  if (std::abs(v2Loaded.params[venom::kParamFM2to1] - 0.0f) > 1e-4f) {
    std::cerr << "v2→v3: FM2to1 default incorrect\n";
    return 17;
  }
  if (std::abs(v2Loaded.params[venom::kParamFM3to1] - 0.0f) > 1e-4f) {
    std::cerr << "v2→v3: FM3to1 default incorrect\n";
    return 18;
  }
  if (std::abs(v2Loaded.params[venom::kParamGrainSize] - 80.0f) > 1e-4f) {
    std::cerr << "v2→v3: GrainSize default incorrect\n";
    return 19;
  }
  if (std::abs(v2Loaded.params[venom::kParamGrainDensity] - 8.0f) > 1e-4f) {
    std::cerr << "v2→v3: GrainDensity default incorrect\n";
    return 20;
  }

  std::cout << "preset migration test passed (v1→v3, v2→v3)\n";
  return 0;
}
