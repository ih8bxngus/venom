#include <iostream>

#include "../src/preset/PresetManager.hpp"

int main() {
  venom::preset::Preset preset;

  const char* missingParams = R"({"version":1,"name":"Broken"})";
  if (venom::preset::PresetManager::fromJson(missingParams, preset)) {
    std::cerr << "parser accepted JSON missing params\n";
    return 1;
  }

  const char* wrongCount = R"({"version":1,"name":"Broken","params":[1,2,3]})";
  if (venom::preset::PresetManager::fromJson(wrongCount, preset)) {
    std::cerr << "parser accepted wrong parameter count\n";
    return 2;
  }

  const char* badNumber = R"({"version":1,"name":"Broken","params":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,wat]})";
  if (venom::preset::PresetManager::fromJson(badNumber, preset)) {
    std::cerr << "parser accepted malformed numeric token\n";
    return 3;
  }

  std::cout << "preset invalid-json test passed\n";
  return 0;
}
