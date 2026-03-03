#include "NanoKnob.hpp"
#include <cmath>
#include <cstdio>
#include <algorithm>

START_NAMESPACE_DGL

static constexpr float kPI = 3.14159265358979323846f;
// Arc sweep: 270 degrees from 7 o'clock to 5 o'clock
static constexpr float kStartAngle = 0.75f * kPI;   // 135 degrees (7 o'clock)
static constexpr float kSweepAngle = 1.5f * kPI;    // 270 degrees
static constexpr float kArcWidth = 3.0f;             // stroke width

NanoKnob::NanoKnob(NanoTopLevelWidget* parent)
    : NanoSubWidget(parent),
      KnobEventHandler(this),
      accentColor_(venom::theme::oscAccent())
{
    setOrientation(KnobEventHandler::Vertical);
    setMouseDeceleration(200.0f);
}

void NanoKnob::setParamIndex(uint32_t index)   { paramIndex_ = index; }
uint32_t NanoKnob::getParamIndex() const       { return paramIndex_; }
void NanoKnob::setLabel(const char* label)     { label_ = label; }
void NanoKnob::setAccentColor(const Color& c)  { accentColor_ = c; }
void NanoKnob::setDisplayFormat(DisplayFormat f){ displayFormat_ = f; }
void NanoKnob::setKnobRadius(float r)          { knobRadius_ = r; }

void NanoKnob::onNanoDisplay()
{
    const float w = getWidth();
    const float cx = w * 0.5f;
    const float cy = knobRadius_ + 2.0f;
    const float r = knobRadius_;
    const float normalized = getNormalizedValue();

    drawArcTrack(cx, cy, r);
    drawArcValue(cx, cy, r, normalized);
    drawIndicator(cx, cy, r, normalized);

    float textY = cy + r + 4.0f;
    drawLabel(cx, textY);
    drawValue(cx, textY + 11.0f);
}

void NanoKnob::drawArcTrack(float cx, float cy, float r)
{
    beginPath();
    arc(cx, cy, r, kStartAngle, kStartAngle + kSweepAngle, NanoVG::CW);
    strokeColor(venom::theme::knobTrack());
    strokeWidth(kArcWidth);
    stroke();
}

void NanoKnob::drawArcValue(float cx, float cy, float r, float normalized)
{
    if (normalized < 0.005f)
        return;

    float endAngle = kStartAngle + normalized * kSweepAngle;

    beginPath();
    arc(cx, cy, r, kStartAngle, endAngle, NanoVG::CW);
    strokeColor(accentColor_);
    strokeWidth(kArcWidth);
    stroke();
}

void NanoKnob::drawIndicator(float cx, float cy, float r, float normalized)
{
    float angle = kStartAngle + normalized * kSweepAngle;
    float ix = cx + std::cos(angle) * (r - 5.0f);
    float iy = cy + std::sin(angle) * (r - 5.0f);
    float ox = cx + std::cos(angle) * (r + 1.0f);
    float oy = cy + std::sin(angle) * (r + 1.0f);

    beginPath();
    moveTo(ix, iy);
    lineTo(ox, oy);
    strokeColor(venom::theme::textPrimary());
    strokeWidth(2.0f);
    stroke();

    // Center dot
    beginPath();
    circle(cx, cy, 3.0f);
    fillColor(venom::theme::knobCenter());
    fill();
    strokeColor(Color::fromHTML("#2A2A3E"));
    strokeWidth(1.0f);
    stroke();
}

void NanoKnob::drawLabel(float cx, float belowY)
{
    if (label_.empty())
        return;

    fontSize(9.0f);
    fillColor(venom::theme::textDim());
    textAlign(ALIGN_CENTER | ALIGN_TOP);
    text(cx, belowY, label_.c_str(), nullptr);
}

void NanoKnob::drawValue(float cx, float belowY)
{
    const char* txt = formatValue(getValue());
    fontSize(9.0f);
    fillColor(venom::theme::textPrimary());
    textAlign(ALIGN_CENTER | ALIGN_TOP);
    text(cx, belowY, txt, nullptr);
}

const char* NanoKnob::formatValue(float value)
{
    switch (displayFormat_) {
        case kFormatFloat1:
            std::snprintf(valueBuf_, sizeof(valueBuf_), "%.1f", value);
            break;
        case kFormatFloat2:
            std::snprintf(valueBuf_, sizeof(valueBuf_), "%.2f", value);
            break;
        case kFormatInt:
            std::snprintf(valueBuf_, sizeof(valueBuf_), "%d", static_cast<int>(std::round(value)));
            break;
        case kFormatHz:
            if (value >= 1000.0f)
                std::snprintf(valueBuf_, sizeof(valueBuf_), "%.1fk", value / 1000.0f);
            else
                std::snprintf(valueBuf_, sizeof(valueBuf_), "%.0f Hz", value);
            break;
        case kFormatMs:
            if (value >= 1.0f)
                std::snprintf(valueBuf_, sizeof(valueBuf_), "%.2fs", value);
            else
                std::snprintf(valueBuf_, sizeof(valueBuf_), "%.0fms", value * 1000.0f);
            break;
        case kFormatPercent:
            std::snprintf(valueBuf_, sizeof(valueBuf_), "%.0f%%", value * 100.0f);
            break;
        case kFormatSemitone:
            std::snprintf(valueBuf_, sizeof(valueBuf_), "%+d st",
                          static_cast<int>(std::round(value)));
            break;
        case kFormatCents:
            std::snprintf(valueBuf_, sizeof(valueBuf_), "%+.0f ct", value);
            break;
    }
    return valueBuf_;
}

bool NanoKnob::onMouse(const MouseEvent& ev)
{
    return KnobEventHandler::mouseEvent(ev);
}

bool NanoKnob::onMotion(const MotionEvent& ev)
{
    return KnobEventHandler::motionEvent(ev);
}

bool NanoKnob::onScroll(const ScrollEvent& ev)
{
    return KnobEventHandler::scrollEvent(ev);
}

END_NAMESPACE_DGL
