#pragma once

#include <array>
#include <vector>

#include "DistrhoPlugin.hpp"
#include "VenomShared.hpp"
#include "core/SynthEngine.hpp"

class VenomPlugin final : public DISTRHO::Plugin {
public:
  VenomPlugin();

protected:
  const char* getLabel() const noexcept override;
  const char* getDescription() const override;
  const char* getMaker() const noexcept override;
  const char* getHomePage() const noexcept override;
  const char* getLicense() const noexcept override;
  uint32_t getVersion() const noexcept override;
  int64_t getUniqueId() const noexcept override;

  void initParameter(uint32_t index, Parameter& parameter) override;
  float getParameterValue(uint32_t index) const override;
  void setParameterValue(uint32_t index, float value) override;

  void activate() override;
  void run(const float** inputs, float** outputs, uint32_t frames, const MidiEvent* midiEvents, uint32_t midiEventCount) override;

private:
  void syncParameters();

  std::array<float, venom::kParameterCount> params_{};
  venom::core::SynthEngine engine_{};
  std::vector<venom::core::MidiEvent> midiScratch_{};
};
