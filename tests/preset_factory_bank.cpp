#include <filesystem>
#include <iostream>
#include <string>

#include "../src/preset/PresetManager.hpp"

int main() {
  const std::filesystem::path presetsDir = "../presets";
  if (!std::filesystem::exists(presetsDir)) {
    std::cerr << "presets directory missing\n";
    return 1;
  }

  size_t jsonCount = 0;
  for (const auto& entry : std::filesystem::directory_iterator(presetsDir)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".json")
      continue;

    ++jsonCount;

    venom::preset::Preset preset;
    if (!venom::preset::PresetManager::loadFromFile(entry.path().string(), preset)) {
      std::cerr << "failed to parse preset: " << entry.path() << "\n";
      return 2;
    }

    if (preset.name.empty()) {
      std::cerr << "preset name empty: " << entry.path() << "\n";
      return 3;
    }
  }

  if (jsonCount < 10) {
    std::cerr << "expected at least 10 factory presets, got " << jsonCount << "\n";
    return 4;
  }

  std::cout << "factory preset bank test passed with " << jsonCount << " files\n";
  return 0;
}
