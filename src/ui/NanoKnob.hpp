#pragma once

#include "NanoVG.hpp"
#include "EventHandlers.hpp"
#include "VenomTheme.hpp"

#include <string>
#include <cstdio>

START_NAMESPACE_DGL

class NanoKnob : public NanoSubWidget,
                 public KnobEventHandler
{
public:
    enum DisplayFormat {
        kFormatFloat1,    // "0.5"
        kFormatFloat2,    // "0.50"
        kFormatInt,       // "12"
        kFormatHz,        // "1.2k" for >1000
        kFormatMs,        // "100ms"
        kFormatPercent,   // "50%"
        kFormatSemitone,  // "+7 st"
        kFormatCents,     // "-25 ct"
    };

    explicit NanoKnob(NanoTopLevelWidget* parent);

    void setParamIndex(uint32_t index);
    uint32_t getParamIndex() const;

    void setLabel(const char* label);
    void setAccentColor(const Color& color);
    void setDisplayFormat(DisplayFormat fmt);
    void setKnobRadius(float radius);

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent& ev) override;
    bool onMotion(const MotionEvent& ev) override;
    bool onScroll(const ScrollEvent& ev) override;

private:
    uint32_t paramIndex_ = 0;
    std::string label_;
    Color accentColor_;
    DisplayFormat displayFormat_ = kFormatFloat1;
    float knobRadius_ = 18.0f;

    void drawArcTrack(float cx, float cy, float r);
    void drawArcValue(float cx, float cy, float r, float normalized);
    void drawIndicator(float cx, float cy, float r, float normalized);
    void drawLabel(float cx, float belowY);
    void drawValue(float cx, float belowY);
    const char* formatValue(float value);
    char valueBuf_[32] = {};

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NanoKnob)
};

END_NAMESPACE_DGL
