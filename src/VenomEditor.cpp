#include "VenomEditor.hpp"

#include "Color.hpp"
#include "Geometry.hpp"
#include <algorithm>

START_NAMESPACE_DISTRHO

VenomEditor::VenomEditor()
    : UI(840, 480) {}

void VenomEditor::parameterChanged(uint32_t, float value) {
  meterLevel_ = value;
  repaint();
}

void VenomEditor::onDisplay() {
  glClearColor(0.04f, 0.04f, 0.07f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  const float glowStrength = 0.15f + (meterLevel_ * 0.85f);
  glColor4f(0.2f + glowStrength, 0.05f, 0.45f + glowStrength, 0.55f);

  Rectangle<float> meterRect(760.0f, 40.0f, 36.0f, 400.0f * std::max(0.05f, meterLevel_));
  meterRect.draw();
}

UI* createUI() {
  return new VenomEditor();
}

END_NAMESPACE_DISTRHO
