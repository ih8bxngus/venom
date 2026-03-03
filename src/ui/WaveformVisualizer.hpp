#pragma once

#include "NanoVG.hpp"
#include "VenomTheme.hpp"

#include <array>
#include <cmath>

START_NAMESPACE_DGL

class WaveformVisualizer : public NanoSubWidget
{
public:
    explicit WaveformVisualizer(NanoTopLevelWidget* parent);

    void setOscWaveform(int oscIndex, int waveformType);
    void setOscWavePos(int oscIndex, float pos);
    void setOscLevel(int oscIndex, float level);

protected:
    void onNanoDisplay() override;

private:
    struct OscState {
        int waveform = 0;    // 0=Sine, 1=Saw, 2=Square, 3=Triangle, 4=Wavetable
        float wavePos = 0.0f;
        float level = 0.0f;
    };

    std::array<OscState, 3> oscState_{};

    void drawPanel(float x, float y, float w, float h, int oscIndex);
    void drawWaveform(float x, float y, float w, float h, int waveform, float wavePos);

    static constexpr int kNumPoints = 128;
    static constexpr float kPanelGap = 8.0f;
    static constexpr float kPanelPad = 6.0f;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformVisualizer)
};

END_NAMESPACE_DGL
