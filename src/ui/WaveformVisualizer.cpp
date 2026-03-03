#include "WaveformVisualizer.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

START_NAMESPACE_DGL

using namespace venom::theme;

WaveformVisualizer::WaveformVisualizer(NanoTopLevelWidget* parent)
    : NanoSubWidget(parent)
{
}

void WaveformVisualizer::setOscWaveform(int oscIndex, int waveformType)
{
    if (oscIndex >= 0 && oscIndex < 3) {
        oscState_[oscIndex].waveform = waveformType;
        repaint();
    }
}

void WaveformVisualizer::setOscWavePos(int oscIndex, float pos)
{
    if (oscIndex >= 0 && oscIndex < 3) {
        oscState_[oscIndex].wavePos = pos;
        repaint();
    }
}

void WaveformVisualizer::setOscLevel(int oscIndex, float level)
{
    if (oscIndex >= 0 && oscIndex < 3) {
        oscState_[oscIndex].level = level;
        repaint();
    }
}

void WaveformVisualizer::onNanoDisplay()
{
    const float totalW = getWidth();
    const float totalH = getHeight();
    const float panelW = (totalW - kPanelGap * 2.0f) / 3.0f;

    for (int i = 0; i < 3; ++i) {
        float px = i * (panelW + kPanelGap);
        drawPanel(px, 0, panelW, totalH, i);
    }
}

void WaveformVisualizer::drawPanel(float x, float y, float w, float h, int oscIndex)
{
    const Color accent = oscAccent();
    const auto& state = oscState_[oscIndex];
    const float alpha = (state.level > 0.001f) ? 1.0f : 0.3f;

    // Panel background
    beginPath();
    roundedRect(x, y, w, h, 4.0f);
    fillColor(sectionBg());
    fill();

    // Subtle top accent line
    beginPath();
    moveTo(x + 4.0f, y);
    lineTo(x + w - 4.0f, y);
    strokeColor(Color(accent.red, accent.green, accent.blue, 0.3f));
    strokeWidth(1.0f);
    stroke();

    // Label
    const char* labels[] = {"OSC 1", "OSC 2", "OSC 3"};
    fontSize(9.0f);
    fillColor(Color(accent.red, accent.green, accent.blue, 0.6f * alpha));
    textAlign(ALIGN_LEFT | ALIGN_TOP);
    text(x + 6.0f, y + 3.0f, labels[oscIndex], nullptr);

    // Waveform display area (with padding)
    float wx = x + kPanelPad;
    float wy = y + 16.0f;
    float ww = w - kPanelPad * 2.0f;
    float wh = h - 22.0f;

    // Draw zero line
    beginPath();
    moveTo(wx, wy + wh * 0.5f);
    lineTo(wx + ww, wy + wh * 0.5f);
    strokeColor(Color(30, 30, 48));
    strokeWidth(1.0f);
    stroke();

    // Draw waveform
    drawWaveform(wx, wy, ww, wh, state.waveform, state.wavePos);

    // Apply alpha overlay for dimmed oscillators
    if (alpha < 1.0f) {
        // Draw a semi-transparent overlay to dim the waveform
        // (waveform is already drawn, but with reduced color in stroke)
    }
}

void WaveformVisualizer::drawWaveform(float x, float y, float w, float h,
                                       int waveform, float wavePos)
{
    const Color accent = oscAccent();
    const float cy = y + h * 0.5f;
    const float amplitude = h * 0.4f;

    beginPath();

    for (int i = 0; i <= kNumPoints; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(kNumPoints);
        float sample = 0.0f;

        switch (waveform) {
        case 0: // Sine
            sample = std::sin(2.0f * static_cast<float>(M_PI) * t);
            break;

        case 1: // Saw
            sample = 2.0f * t - 1.0f;
            break;

        case 2: // Square
            sample = (t < 0.5f) ? 1.0f : -1.0f;
            break;

        case 3: // Triangle
            sample = (t < 0.5f) ? (4.0f * t - 1.0f) : (3.0f - 4.0f * t);
            break;

        case 4: { // Wavetable (additive harmonics)
            int numHarmonics = 1 + static_cast<int>(wavePos * 63.0f);
            for (int h = 1; h <= numHarmonics; ++h) {
                sample += (1.0f / static_cast<float>(h))
                          * std::sin(2.0f * static_cast<float>(M_PI) * static_cast<float>(h) * t);
            }
            // Normalize
            if (numHarmonics > 1) {
                float peak = 0.0f;
                for (int j = 0; j <= kNumPoints; ++j) {
                    float tt = static_cast<float>(j) / static_cast<float>(kNumPoints);
                    float s = 0.0f;
                    for (int hh = 1; hh <= numHarmonics; ++hh)
                        s += (1.0f / static_cast<float>(hh))
                             * std::sin(2.0f * static_cast<float>(M_PI) * static_cast<float>(hh) * tt);
                    if (std::abs(s) > peak) peak = std::abs(s);
                }
                if (peak > 0.001f) sample /= peak;
            }
            break;
        }

        default:
            sample = 0.0f;
            break;
        }

        float px = x + t * w;
        float py = cy - sample * amplitude;

        if (i == 0)
            moveTo(px, py);
        else
            lineTo(px, py);
    }

    strokeColor(Color(accent.red, accent.green, accent.blue, 0.9f));
    strokeWidth(1.5f);
    stroke();
}

END_NAMESPACE_DGL
