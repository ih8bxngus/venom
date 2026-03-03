#pragma once

#include "DistrhoUI.hpp"
#include "VenomShared.hpp"
#include "ui/NanoKnob.hpp"
#include "ui/WaveformSelector.hpp"
#include "ui/ToggleSwitch.hpp"
#include "ui/PresetBrowser.hpp"
#include "ui/WaveformVisualizer.hpp"
#include "ui/VirtualKeyboard.hpp"
#include "ui/VenomTheme.hpp"

#include <array>

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::NanoKnob;
using DGL_NAMESPACE::WaveformSelector;
using DGL_NAMESPACE::ToggleSwitch;
using DGL_NAMESPACE::PresetBrowser;
using DGL_NAMESPACE::WaveformVisualizer;
using DGL_NAMESPACE::VirtualKeyboard;
using DGL_NAMESPACE::KnobEventHandler;
using DGL_NAMESPACE::Color;

class VenomEditor final : public UI,
                          public KnobEventHandler::Callback,
                          public WaveformSelector::Listener,
                          public ToggleSwitch::Listener,
                          public PresetBrowser::Listener,
                          public VirtualKeyboard::Listener
{
public:
    VenomEditor();

protected:
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char* key, const char* value) override;
    void onNanoDisplay() override;

    // KnobEventHandler::Callback
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

    // WaveformSelector::Listener
    void waveformChanged(WaveformSelector* selector, int waveIndex) override;

    // ToggleSwitch::Listener
    void toggleChanged(ToggleSwitch* toggle, bool state) override;

    // PresetBrowser::Listener
    void presetSelected(PresetBrowser* browser, int index) override;

    // VirtualKeyboard::Listener
    void keyPressed(VirtualKeyboard* kb, uint8_t note, uint8_t velocity) override;
    void keyReleased(VirtualKeyboard* kb, uint8_t note) override;

private:
    std::array<NanoKnob*, venom::kParameterCount> knobs_{};
    WaveformSelector* oscWave_[3] = {};
    WaveformSelector* lfoWaveSel_ = nullptr;
    ToggleSwitch* envRetrigger_ = nullptr;
    ToggleSwitch* polyToggle_ = nullptr;

    PresetBrowser* presetBrowser_ = nullptr;
    WaveformVisualizer* waveformViz_ = nullptr;
    VirtualKeyboard* keyboard_ = nullptr;

    std::array<float, venom::kParameterCount> paramValues_{};

    NanoKnob* createKnob(uint32_t paramIndex,
                         const char* label,
                         float min, float max, float def,
                         NanoKnob::DisplayFormat fmt,
                         const Color& accent,
                         int x, int y, int w, int h,
                         float radius);

    void drawBackground();
    void drawHeader();
    void drawSectionStrip(float x, float y, float w, const char* title, const Color& accent);
    void drawVerticalDivider(float x, float y1, float y2);
    void updateWaveformVisualizer();

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VenomEditor)
};

END_NAMESPACE_DISTRHO
