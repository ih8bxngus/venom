#pragma once

#include "NanoVG.hpp"
#include "VenomTheme.hpp"

#include <string>

START_NAMESPACE_DGL

class ToggleSwitch : public NanoSubWidget
{
public:
    class Listener {
    public:
        virtual ~Listener() {}
        virtual void toggleChanged(ToggleSwitch* toggle, bool state) = 0;
    };

    explicit ToggleSwitch(NanoTopLevelWidget* parent);

    void setParamIndex(uint32_t index);
    uint32_t getParamIndex() const;

    void setAccentColor(const Color& color);
    void setState(bool on);
    bool getState() const;
    void setLabel(const char* label);

    void setListener(Listener* listener);

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent& ev) override;

private:
    uint32_t paramIndex_ = 0;
    bool state_ = false;
    Color accentColor_;
    std::string label_;
    Listener* listener_ = nullptr;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleSwitch)
};

END_NAMESPACE_DGL
