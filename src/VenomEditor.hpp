#pragma once

#include "DistrhoUI.hpp"

class VenomEditor final : public DISTRHO::UI {
public:
  VenomEditor();

protected:
  void parameterChanged(uint32_t, float) override;
  void onDisplay() override;

private:
  float meterLevel_ = 0.0f;
};
