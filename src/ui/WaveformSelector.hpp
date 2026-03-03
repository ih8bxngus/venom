#pragma once

#include "NanoVG.hpp"
#include "VenomTheme.hpp"

#include <string>
#include <cstdint>

START_NAMESPACE_DGL

class WaveformSelector : public NanoSubWidget
{
public:
    class Listener {
    public:
        virtual ~Listener() {}
        virtual void waveformChanged(WaveformSelector* selector, int waveIndex) = 0;
    };

    explicit WaveformSelector(NanoTopLevelWidget* parent);

    void setParamIndex(uint32_t index);
    uint32_t getParamIndex() const;

    void setAccentColor(const Color& color);
    void setSelectedWave(int waveIndex);
    int getSelectedWave() const;
    void setNumWaveforms(int count);

    void setListener(Listener* listener);

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent& ev) override;

private:
    uint32_t paramIndex_ = 0;
    int selectedWave_ = 0;
    int numWaveforms_ = 5;
    Color accentColor_;
    Listener* listener_ = nullptr;

    void drawSineIcon(float x, float y, float w, float h, bool selected);
    void drawSawIcon(float x, float y, float w, float h, bool selected);
    void drawSquareIcon(float x, float y, float w, float h, bool selected);
    void drawTriangleIcon(float x, float y, float w, float h, bool selected);
    void drawWavetableIcon(float x, float y, float w, float h, bool selected);

    int hitTest(float px, float py) const;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformSelector)
};

END_NAMESPACE_DGL
