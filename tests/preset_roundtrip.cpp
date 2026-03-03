#include <cmath>
#include <filesystem>
#include <iostream>

#include "../src/preset/PresetManager.hpp"

int main() {
  venom::preset::Preset preset;
  preset.version = 1;
  preset.name = "Roundtrip Test";

  for (size_t i = 0; i < preset.params.size(); ++i)
    preset.params[i] = static_cast<float>(i) * 0.37f - 4.0f;

  const std::filesystem::path path = "preset_roundtrip.json";
  if (!venom::preset::PresetManager::saveToFile(preset, path.string())) {
    std::cerr << "failed to save preset file\n";
    return 1;
  }

  venom::preset::Preset loaded;
  if (!venom::preset::PresetManager::loadFromFile(path.string(), loaded)) {
    std::cerr << "failed to load preset file\n";
    return 2;
  }

  if (loaded.name != preset.name || loaded.version != preset.version) {
    std::cerr << "metadata mismatch after roundtrip\n";
    return 3;
  }

  for (size_t i = 0; i < preset.params.size(); ++i) {
    if (std::abs(loaded.params[i] - preset.params[i]) > 1.0e-4f) {
      std::cerr << "param mismatch at index " << i << "\n";
      return 4;
    }
  }

  std::cout << "preset roundtrip test passed\n";
  return 0;
}
