#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace venom::dsp {

class Wavetable {
public:
  static constexpr uint32_t kNumFrames = 64;
  static constexpr uint32_t kFrameSize = 2048;

  Wavetable() { generate(); }

  // Sample the wavetable with bilinear interpolation
  // framePos: 0..1 selects which frame (with interpolation between adjacent)
  // phase: 0..1 position within one cycle
  float sample(float framePos, float phase) const {
    const float frameF = framePos * static_cast<float>(kNumFrames - 1);
    const uint32_t frameA = std::min(static_cast<uint32_t>(frameF), kNumFrames - 1);
    const uint32_t frameB = std::min(frameA + 1, kNumFrames - 1);
    const float frameFrac = frameF - static_cast<float>(frameA);

    const float sA = sampleFrame(frameA, phase);
    const float sB = sampleFrame(frameB, phase);

    return sA + frameFrac * (sB - sA);
  }

private:
  void generate() {
    constexpr float kTwoPi = 6.28318530718f;

    for (uint32_t f = 0; f < kNumFrames; ++f) {
      const uint32_t numHarmonics = f + 1;

      for (uint32_t s = 0; s < kFrameSize; ++s) {
        const float t = static_cast<float>(s) / static_cast<float>(kFrameSize);
        float value = 0.0f;

        for (uint32_t h = 1; h <= numHarmonics; ++h) {
          // Saw-like harmonic series with natural rolloff
          value += std::sin(kTwoPi * static_cast<float>(h) * t) / static_cast<float>(h);
        }

        frames_[f][s] = value;
      }

      // Normalize each frame to [-1, 1]
      float peak = 0.0f;
      for (uint32_t s = 0; s < kFrameSize; ++s)
        peak = std::max(peak, std::abs(frames_[f][s]));

      if (peak > 0.0f) {
        const float scale = 1.0f / peak;
        for (uint32_t s = 0; s < kFrameSize; ++s)
          frames_[f][s] *= scale;
      }
    }
  }

  float sampleFrame(uint32_t frame, float phase) const {
    const float pos = phase * static_cast<float>(kFrameSize);
    const uint32_t idxA = static_cast<uint32_t>(pos) % kFrameSize;
    const uint32_t idxB = (idxA + 1) % kFrameSize;
    const float frac = pos - std::floor(pos);

    return frames_[frame][idxA] + frac * (frames_[frame][idxB] - frames_[frame][idxA]);
  }

  std::array<std::array<float, kFrameSize>, kNumFrames> frames_{};
};

} // namespace venom::dsp
