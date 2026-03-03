#include "ToggleSwitch.hpp"

START_NAMESPACE_DGL

ToggleSwitch::ToggleSwitch(NanoTopLevelWidget* parent)
    : NanoSubWidget(parent),
      accentColor_(venom::theme::lfoAccent())
{
}

void ToggleSwitch::setParamIndex(uint32_t index) { paramIndex_ = index; }
uint32_t ToggleSwitch::getParamIndex() const     { return paramIndex_; }
void ToggleSwitch::setAccentColor(const Color& c){ accentColor_ = c; }
void ToggleSwitch::setLabel(const char* label)   { label_ = label; }
void ToggleSwitch::setListener(Listener* l)      { listener_ = l; }

void ToggleSwitch::setState(bool on) { state_ = on; }
bool ToggleSwitch::getState() const  { return state_; }

void ToggleSwitch::onNanoDisplay()
{
    const float w = getWidth();
    const float h = getHeight();

    // Label above toggle
    if (!label_.empty()) {
        fontSize(9.0f);
        fillColor(venom::theme::textDim());
        textAlign(ALIGN_CENTER | ALIGN_TOP);
        text(w * 0.5f, 0.0f, label_.c_str(), nullptr);
    }

    // Pill toggle (centered horizontally, below label)
    const float pillW = 30.0f;
    const float pillH = 14.0f;
    const float pillX = (w - pillW) * 0.5f;
    const float pillY = h - pillH - 2.0f;
    const float pillR = pillH * 0.5f;

    // Pill background
    beginPath();
    roundedRect(pillX, pillY, pillW, pillH, pillR);
    if (state_) {
        fillColor(Color(accentColor_.red, accentColor_.green, accentColor_.blue, 0.7f));
    } else {
        fillColor(venom::theme::knobTrack());
    }
    fill();

    // Ball
    const float ballR = pillH * 0.5f - 2.0f;
    float ballX = state_ ? (pillX + pillW - pillR) : (pillX + pillR);
    float ballY = pillY + pillH * 0.5f;

    beginPath();
    circle(ballX, ballY, ballR);
    fillColor(state_ ? venom::theme::textPrimary() : venom::theme::textDim());
    fill();
}

bool ToggleSwitch::onMouse(const MouseEvent& ev)
{
    if (!ev.press || ev.button != 1)
        return false;

    if (!contains(ev.pos))
        return false;

    state_ = !state_;
    if (listener_ != nullptr)
        listener_->toggleChanged(this, state_);
    repaint();
    return true;
}

END_NAMESPACE_DGL
