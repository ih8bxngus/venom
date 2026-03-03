#pragma once

#include <array>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../VenomShared.hpp"

namespace venom::preset {

struct Preset {
  std::string name;
  std::array<float, kParameterCount> params{};
  int version = 1;
};

class PresetManager {
public:
  static std::string toJson(const Preset& preset) {
    std::ostringstream os;
    os << "{\n";
    os << "  \"version\": " << preset.version << ",\n";
    os << "  \"name\": \"" << escape(preset.name) << "\",\n";
    os << "  \"params\": [";

    for (size_t i = 0; i < preset.params.size(); ++i) {
      if (i != 0)
        os << ", ";
      os << preset.params[i];
    }

    os << "]\n";
    os << "}\n";
    return os.str();
  }

  static bool fromJson(const std::string& json, Preset& outPreset) {
    const auto namePos = json.find("\"name\"");
    const auto paramsPos = json.find("\"params\"");
    if (namePos == std::string::npos || paramsPos == std::string::npos)
      return false;

    if (!parseName(json, namePos, outPreset.name))
      return false;

    if (!parseParams(json, paramsPos, outPreset.params))
      return false;

    parseVersion(json, outPreset.version);
    return true;
  }

  static bool saveToFile(const Preset& preset, const std::string& path) {
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file)
      return false;

    file << toJson(preset);
    return static_cast<bool>(file);
  }

  static bool loadFromFile(const std::string& path, Preset& outPreset) {
    std::ifstream file(path);
    if (!file)
      return false;

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return fromJson(buffer.str(), outPreset);
  }

private:
  static std::string escape(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (char c : in) {
      if (c == '"' || c == '\\')
        out.push_back('\\');
      out.push_back(c);
    }
    return out;
  }

  static bool parseName(const std::string& json, size_t start, std::string& outName) {
    const auto firstQuote = json.find('"', start + 6);
    if (firstQuote == std::string::npos)
      return false;
    const auto secondQuote = json.find('"', firstQuote + 1);
    if (secondQuote == std::string::npos)
      return false;
    outName = json.substr(firstQuote + 1, secondQuote - firstQuote - 1);
    return true;
  }

  static bool parseParams(const std::string& json, size_t start, std::array<float, kParameterCount>& outParams) {
    const auto openBracket = json.find('[', start);
    const auto closeBracket = json.find(']', openBracket);
    if (openBracket == std::string::npos || closeBracket == std::string::npos)
      return false;

    const std::string body = json.substr(openBracket + 1, closeBracket - openBracket - 1);
    std::stringstream ss(body);
    std::string token;
    std::vector<float> parsed;

    while (std::getline(ss, token, ',')) {
      trim(token);
      if (token.empty())
        continue;
      try {
        parsed.push_back(std::stof(token));
      } catch (...) {
        return false;
      }
    }

    if (parsed.size() == kOldParameterCount_v1) {
      // Backward-compatible: load old 29-param preset with sensible defaults
      outParams.fill(0.0f);
      for (size_t i = 0; i < kOldParameterCount_v1; ++i)
        outParams[i] = parsed[i];

      // Fill in v0.4 defaults for missing params
      outParams[kParamPolyMode]      = 1.0f;   // Poly on
      outParams[kParamUnisonCount]   = 1.0f;   // No unison
      outParams[kParamUnisonDetune]  = 15.0f;  // 15 cents
      outParams[kParamUnisonSpread]  = 0.5f;   // 50% stereo
      outParams[kParamOsc1WavePos]   = 0.0f;
      outParams[kParamOsc2WavePos]   = 0.0f;
      outParams[kParamOsc3WavePos]   = 0.0f;
      outParams[kParamLfoWave]       = 0.0f;   // Sine LFO
      // v0.5 defaults applied below
      applyV3Defaults(outParams);
      return true;
    }

    if (parsed.size() == kOldParameterCount_v2) {
      // Backward-compatible: load 37-param (v0.4) preset with v0.5 defaults
      outParams.fill(0.0f);
      for (size_t i = 0; i < kOldParameterCount_v2; ++i)
        outParams[i] = parsed[i];

      applyV3Defaults(outParams);
      return true;
    }

    if (parsed.size() != outParams.size())
      return false;

    for (size_t i = 0; i < outParams.size(); ++i)
      outParams[i] = parsed[i];

    return true;
  }

  static void parseVersion(const std::string& json, int& outVersion) {
    const auto pos = json.find("\"version\"");
    if (pos == std::string::npos)
      return;
    const auto colon = json.find(':', pos);
    if (colon == std::string::npos)
      return;

    size_t i = colon + 1;
    while (i < json.size() && std::isspace(static_cast<unsigned char>(json[i])))
      ++i;

    std::string digits;
    while (i < json.size() && std::isdigit(static_cast<unsigned char>(json[i]))) {
      digits.push_back(json[i]);
      ++i;
    }

    if (!digits.empty())
      outVersion = std::stoi(digits);
  }

  static void applyV3Defaults(std::array<float, kParameterCount>& params) {
    // v0.5 Phase 1: FM amounts default to 0 (no modulation)
    params[kParamFM2to1]         = 0.0f;
    params[kParamFM3to1]         = 0.0f;
    params[kParamFM3to2]         = 0.0f;
    // v0.5 Phase 2: Wavetable banks default to bank 0
    params[kParamOsc1WtBank]     = 0.0f;
    params[kParamOsc2WtBank]     = 0.0f;
    params[kParamOsc3WtBank]     = 0.0f;
    // v0.5 Phase 3: Classic engine, sensible grain defaults
    params[kParamOsc1Engine]     = 0.0f;   // Classic
    params[kParamOsc2Engine]     = 0.0f;
    params[kParamOsc3Engine]     = 0.0f;
    params[kParamGrainSize]      = 80.0f;  // 80ms
    params[kParamGrainDensity]   = 8.0f;   // 8 grains/sec
    params[kParamGrainScatter]   = 0.1f;
    params[kParamGrainPitchRand] = 0.0f;
    params[kParamSampleStart]    = 0.0f;
    params[kParamSampleSelect]   = 0.0f;
    params[kParamGrainWindow]    = 0.0f;   // Hann
  }

  static void trim(std::string& s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
      s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
      s.pop_back();
  }
};

} // namespace venom::preset
