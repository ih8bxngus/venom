#pragma once

#include <algorithm>
#include <cmath>

namespace venom::dsp {

class BiquadLowpass {
public:
  void setSampleRate(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    updateCoefficients();
  }

  void setCutoff(float cutoffHz) {
    cutoffHz_ = std::clamp(cutoffHz, 20.0f, 20000.0f);
    updateCoefficients();
  }

  void setResonance(float resonance) {
    resonance_ = std::clamp(resonance, 0.0f, 1.0f);
    updateCoefficients();
  }

  float process(float input) {
    const float output = b0_ * input + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;
    x2_ = x1_;
    x1_ = input;
    y2_ = y1_;
    y1_ = output;
    return output;
  }

  void reset() { x1_ = x2_ = y1_ = y2_ = 0.0f; }

private:
  void updateCoefficients() {
    const float clampedCutoff = std::min(cutoffHz_, static_cast<float>(sampleRate_ * 0.49));
    const float omega = 2.0f * 3.14159265359f * clampedCutoff / static_cast<float>(sampleRate_);
    const float sinOmega = std::sin(omega);
    const float cosOmega = std::cos(omega);

    const float q = 0.5f + (resonance_ * 9.5f);
    const float alpha = sinOmega / (2.0f * q);

    const float a0 = 1.0f + alpha;
    b0_ = ((1.0f - cosOmega) * 0.5f) / a0;
    b1_ = (1.0f - cosOmega) / a0;
    b2_ = ((1.0f - cosOmega) * 0.5f) / a0;
    a1_ = (-2.0f * cosOmega) / a0;
    a2_ = (1.0f - alpha) / a0;
  }

  double sampleRate_ = 44100.0;
  float cutoffHz_ = 1200.0f;
  float resonance_ = 0.0f;

  float b0_ = 0.0f;
  float b1_ = 0.0f;
  float b2_ = 0.0f;
  float a1_ = 0.0f;
  float a2_ = 0.0f;

  float x1_ = 0.0f;
  float x2_ = 0.0f;
  float y1_ = 0.0f;
  float y2_ = 0.0f;
};

class Lowpass24 {
public:
  void setSampleRate(double sampleRate) {
    stage1_.setSampleRate(sampleRate);
    stage2_.setSampleRate(sampleRate);
  }

  void setCutoff(float cutoffHz) {
    stage1_.setCutoff(cutoffHz);
    stage2_.setCutoff(cutoffHz);
  }

  void setResonance(float resonance) {
    stage1_.setResonance(resonance);
    stage2_.setResonance(resonance);
  }

  float process(float input) {
    return stage2_.process(stage1_.process(input));
  }

  void reset() {
    stage1_.reset();
    stage2_.reset();
  }

private:
  BiquadLowpass stage1_;
  BiquadLowpass stage2_;
};

} // namespace venom::dsp
