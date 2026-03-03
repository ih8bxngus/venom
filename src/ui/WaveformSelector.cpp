#include "WaveformSelector.hpp"
#include <cmath>

START_NAMESPACE_DGL

static constexpr float kPI = 3.14159265358979323846f;

WaveformSelector::WaveformSelector(NanoTopLevelWidget* parent)
    : NanoSubWidget(parent),
      accentColor_(venom::theme::oscAccent())
{
}

void WaveformSelector::setParamIndex(uint32_t index) { paramIndex_ = index; }
uint32_t WaveformSelector::getParamIndex() const     { return paramIndex_; }
void WaveformSelector::setAccentColor(const Color& c){ accentColor_ = c; }
void WaveformSelector::setListener(Listener* l)      { listener_ = l; }

void WaveformSelector::setSelectedWave(int waveIndex)
{
    if (waveIndex >= 0 && waveIndex < numWaveforms_)
        selectedWave_ = waveIndex;
}

int WaveformSelector::getSelectedWave() const { return selectedWave_; }

void WaveformSelector::setNumWaveforms(int count)
{
    numWaveforms_ = (count >= 1 && count <= 5) ? count : 5;
    if (selectedWave_ >= numWaveforms_)
        selectedWave_ = 0;
}

void WaveformSelector::onNanoDisplay()
{
    const float w = getWidth();
    const float h = getHeight();
    const float cellW = w / static_cast<float>(numWaveforms_);
    const float pad = 3.0f;
    const float iconH = h - pad * 2.0f;
    const float iconW = cellW - pad * 2.0f;

    for (int i = 0; i < numWaveforms_; ++i) {
        float x = i * cellW + pad;
        float y = pad;
        bool sel = (i == selectedWave_);

        // Background highlight for selected
        if (sel) {
            beginPath();
            roundedRect(x - 1.0f, y - 1.0f, iconW + 2.0f, iconH + 2.0f, 3.0f);
            fillColor(Color(accentColor_.red, accentColor_.green, accentColor_.blue, 0.15f));
            fill();
        }

        switch (i) {
            case 0: drawSineIcon(x, y, iconW, iconH, sel); break;
            case 1: drawSawIcon(x, y, iconW, iconH, sel); break;
            case 2: drawSquareIcon(x, y, iconW, iconH, sel); break;
            case 3: drawTriangleIcon(x, y, iconW, iconH, sel); break;
            case 4: drawWavetableIcon(x, y, iconW, iconH, sel); break;
        }
    }
}

void WaveformSelector::drawSineIcon(float x, float y, float w, float h, bool selected)
{
    const float cy = y + h * 0.5f;
    const float amp = h * 0.35f;
    const int steps = 20;

    beginPath();
    for (int i = 0; i <= steps; ++i) {
        float t = static_cast<float>(i) / steps;
        float px = x + t * w;
        float py = cy - amp * std::sin(t * 2.0f * kPI);
        if (i == 0) moveTo(px, py);
        else lineTo(px, py);
    }
    strokeColor(selected ? accentColor_ : venom::theme::textDim());
    strokeWidth(1.5f);
    stroke();
}

void WaveformSelector::drawSawIcon(float x, float y, float w, float h, bool selected)
{
    const float cy = y + h * 0.5f;
    const float amp = h * 0.35f;

    beginPath();
    moveTo(x, cy);
    lineTo(x + w, cy - amp);
    lineTo(x + w, cy + amp);
    strokeColor(selected ? accentColor_ : venom::theme::textDim());
    strokeWidth(1.5f);
    stroke();
}

void WaveformSelector::drawSquareIcon(float x, float y, float w, float h, bool selected)
{
    const float cy = y + h * 0.5f;
    const float amp = h * 0.35f;
    const float halfW = w * 0.5f;

    beginPath();
    moveTo(x, cy + amp);
    lineTo(x, cy - amp);
    lineTo(x + halfW, cy - amp);
    lineTo(x + halfW, cy + amp);
    lineTo(x + w, cy + amp);
    strokeColor(selected ? accentColor_ : venom::theme::textDim());
    strokeWidth(1.5f);
    stroke();
}

void WaveformSelector::drawTriangleIcon(float x, float y, float w, float h, bool selected)
{
    const float cy = y + h * 0.5f;
    const float amp = h * 0.35f;

    beginPath();
    moveTo(x, cy);
    lineTo(x + w * 0.25f, cy - amp);
    lineTo(x + w * 0.75f, cy + amp);
    lineTo(x + w, cy);
    strokeColor(selected ? accentColor_ : venom::theme::textDim());
    strokeWidth(1.5f);
    stroke();
}

void WaveformSelector::drawWavetableIcon(float x, float y, float w, float h, bool selected)
{
    // Complex wavy line (multiple harmonics overlaid) to represent wavetable/S&H
    const float cy = y + h * 0.5f;
    const float amp = h * 0.30f;
    const int steps = 24;

    beginPath();
    for (int i = 0; i <= steps; ++i) {
        float t = static_cast<float>(i) / steps;
        float px = x + t * w;
        // Complex waveform: fundamental + 3rd harmonic + 5th harmonic
        float py = cy - amp * (0.6f * std::sin(t * 2.0f * kPI) +
                                0.3f * std::sin(t * 6.0f * kPI) +
                                0.15f * std::sin(t * 10.0f * kPI));
        if (i == 0) moveTo(px, py);
        else lineTo(px, py);
    }
    strokeColor(selected ? accentColor_ : venom::theme::textDim());
    strokeWidth(1.5f);
    stroke();
}

int WaveformSelector::hitTest(float px, float py) const
{
    const float cellW = getWidth() / static_cast<float>(numWaveforms_);
    if (py < 0 || py > static_cast<float>(getHeight()))
        return -1;
    if (px < 0 || px > static_cast<float>(getWidth()))
        return -1;
    int idx = static_cast<int>(px / cellW);
    return (idx < numWaveforms_) ? idx : -1;
}

bool WaveformSelector::onMouse(const MouseEvent& ev)
{
    if (!ev.press || ev.button != 1)
        return false;

    if (!contains(ev.pos))
        return false;

    int idx = hitTest(static_cast<float>(ev.pos.getX()),
                      static_cast<float>(ev.pos.getY()));
    if (idx >= 0 && idx < numWaveforms_ && idx != selectedWave_) {
        selectedWave_ = idx;
        if (listener_ != nullptr)
            listener_->waveformChanged(this, idx);
        repaint();
        return true;
    }
    return false;
}

END_NAMESPACE_DGL
