#include "VenomEditor.hpp"
#include "preset/FactoryPresets.hpp"

#include <cstring>
#include <cstdio>

START_NAMESPACE_DISTRHO

using namespace venom;
using namespace venom::theme;

// ---------------------------------------------------------------------------
// Layout: Pigments-inspired horizontal strips
//
// Header:       y=0,    h=34
// Oscillators:  y=34,   h=134  (3 columns)
// FM Matrix:    y=168,  h=50   (3 knobs — FM routing)
// Filter+AmpE:  y=218,  h=106  (2 columns)
// FltEnv+LFO+M: y=324,  h=106  (3 columns, VOICE merged into MASTER)
// Divider:      y=430
// Waveform Viz: y=434,  h=80
// Keyboard:     y=518,  h=120
// Footer pad:   to 790

static constexpr float kW = 960.0f;
static constexpr float kH = 790.0f;

// Header
static constexpr float kHeaderH = 34.0f;

// Oscillators strip
static constexpr float kOscY = 34.0f;
static constexpr float kOscH = 134.0f;
static constexpr float kOscColW = 320.0f; // 960 / 3

// FM Matrix strip (new in v0.5)
static constexpr float kFmY = 168.0f;
static constexpr float kFmH = 50.0f;

// Filter + Amp Env strip
static constexpr float kFiltY = 218.0f;
static constexpr float kFiltH = 106.0f;
static constexpr float kFiltColW = 480.0f;
static constexpr float kAmpEnvColW = 480.0f;

// Flt Env + LFO + Master strip
static constexpr float kRow3Y = 324.0f;
static constexpr float kRow3H = 106.0f;
static constexpr float kFltEnvColW = 320.0f;
static constexpr float kLfoColW = 320.0f;
static constexpr float kMasterColW = 320.0f;

// Waveform viz
static constexpr float kVizY = 434.0f;
static constexpr float kVizH = 80.0f;

// Keyboard
static constexpr float kKbdY = 518.0f;
static constexpr float kKbdH = 120.0f;

// Margins
static constexpr float kMargin = 8.0f;
static constexpr float kInnerPad = 10.0f;

// Knob sizes
static constexpr int kKnobW = 48;
static constexpr int kKnobH = 52;
static constexpr float kKnobR = 16.0f;
static constexpr float kKnobRLarge = 20.0f;
static constexpr int kWaveSelH = 22;

// ---------------------------------------------------------------------------

NanoKnob* VenomEditor::createKnob(uint32_t paramIndex,
                                   const char* label,
                                   float min, float max, float def,
                                   NanoKnob::DisplayFormat fmt,
                                   const Color& accent,
                                   int x, int y, int w, int h,
                                   float radius)
{
    NanoKnob* knob = new NanoKnob(this);
    knob->setParamIndex(paramIndex);
    knob->setLabel(label);
    knob->setAccentColor(accent);
    knob->setDisplayFormat(fmt);
    knob->setKnobRadius(radius);
    knob->setRange(min, max);
    knob->setDefault(def);
    knob->setValue(def, false);
    knob->setCallback(this);
    knob->setAbsolutePos(x, y);
    knob->setSize(w, h);
    knobs_[paramIndex] = knob;
    return knob;
}

// ---------------------------------------------------------------------------

VenomEditor::VenomEditor()
    : UI(960, 790)
{
    loadSharedResources();

    paramValues_.fill(0.0f);
    knobs_.fill(nullptr);

    const Color purple = oscAccent();
    const Color cyan   = filterAccent();
    const Color green  = envAccent();
    const Color orange = lfoAccent();

    // ========= PRESET BROWSER (header) =========
    {
        presetBrowser_ = new PresetBrowser(this);
        presetBrowser_->setAbsolutePos(160, 5);
        presetBrowser_->setSize(520, 24);
        presetBrowser_->setPresetCount(static_cast<int>(preset::kFactoryPresetCount));
        presetBrowser_->setPresetName(preset::kFactoryPresets[0].name);
        for (size_t i = 0; i < preset::kFactoryPresetCount; ++i)
            presetBrowser_->addPreset(static_cast<int>(i),
                                      preset::kFactoryPresets[i].name,
                                      preset::kFactoryPresets[i].category);
        presetBrowser_->setListener(this);
    }

    // ========= OSCILLATOR 1 (top-left column) =========
    {
        float sx = kMargin + kInnerPad;
        float sy = kOscY + 18.0f;

        oscWave_[0] = new WaveformSelector(this);
        oscWave_[0]->setParamIndex(kParamOsc1Wave);
        oscWave_[0]->setNumWaveforms(5);
        oscWave_[0]->setAccentColor(purple);
        oscWave_[0]->setListener(this);
        oscWave_[0]->setAbsolutePos(static_cast<int>(sx), static_cast<int>(sy));
        oscWave_[0]->setSize(static_cast<uint>(kOscColW - kInnerPad * 2 - kMargin), kWaveSelH);

        float ky = sy + kWaveSelH + 6.0f;
        float gap = 52.0f;
        createKnob(kParamOsc1Detune, "Detune", -50, 50, 0, NanoKnob::kFormatCents, purple,
                   static_cast<int>(sx), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamOsc1Pitch, "Pitch", -48, 48, 0, NanoKnob::kFormatSemitone, purple,
                   static_cast<int>(sx + gap), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamOsc1Level, "Level", 0, 1, 0.6f, NanoKnob::kFormatPercent, purple,
                   static_cast<int>(sx + gap * 2), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamOsc1WavePos, "WT Pos", 0, 1, 0, NanoKnob::kFormatPercent, purple,
                   static_cast<int>(sx + gap * 3), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
    }

    // ========= OSCILLATOR 2 (top-center column) =========
    {
        float sx = kOscColW + kInnerPad;
        float sy = kOscY + 18.0f;

        oscWave_[1] = new WaveformSelector(this);
        oscWave_[1]->setParamIndex(kParamOsc2Wave);
        oscWave_[1]->setNumWaveforms(5);
        oscWave_[1]->setAccentColor(purple);
        oscWave_[1]->setListener(this);
        oscWave_[1]->setAbsolutePos(static_cast<int>(sx), static_cast<int>(sy));
        oscWave_[1]->setSize(static_cast<uint>(kOscColW - kInnerPad * 2), kWaveSelH);

        float ky = sy + kWaveSelH + 6.0f;
        float gap = 52.0f;
        createKnob(kParamOsc2Detune, "Detune", -50, 50, 0, NanoKnob::kFormatCents, purple,
                   static_cast<int>(sx), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamOsc2Pitch, "Pitch", -48, 48, 0, NanoKnob::kFormatSemitone, purple,
                   static_cast<int>(sx + gap), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamOsc2Level, "Level", 0, 1, 0.4f, NanoKnob::kFormatPercent, purple,
                   static_cast<int>(sx + gap * 2), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamOsc2WavePos, "WT Pos", 0, 1, 0, NanoKnob::kFormatPercent, purple,
                   static_cast<int>(sx + gap * 3), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
    }

    // ========= OSCILLATOR 3 (top-right column) =========
    {
        float sx = kOscColW * 2 + kInnerPad;
        float sy = kOscY + 18.0f;

        oscWave_[2] = new WaveformSelector(this);
        oscWave_[2]->setParamIndex(kParamOsc3Wave);
        oscWave_[2]->setNumWaveforms(5);
        oscWave_[2]->setAccentColor(purple);
        oscWave_[2]->setListener(this);
        oscWave_[2]->setAbsolutePos(static_cast<int>(sx), static_cast<int>(sy));
        oscWave_[2]->setSize(static_cast<uint>(kOscColW - kInnerPad * 2 - kMargin), kWaveSelH);

        float ky = sy + kWaveSelH + 6.0f;
        float gap = 52.0f;
        createKnob(kParamOsc3Detune, "Detune", -50, 50, 0, NanoKnob::kFormatCents, purple,
                   static_cast<int>(sx), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamOsc3Pitch, "Pitch", -48, 48, 0, NanoKnob::kFormatSemitone, purple,
                   static_cast<int>(sx + gap), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamOsc3Level, "Level", 0, 1, 0.35f, NanoKnob::kFormatPercent, purple,
                   static_cast<int>(sx + gap * 2), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamOsc3WavePos, "WT Pos", 0, 1, 0, NanoKnob::kFormatPercent, purple,
                   static_cast<int>(sx + gap * 3), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
    }

    // ========= FM MATRIX (between oscillators and filter) =========
    {
        const Color fmColor = fmAccent();
        float sx = kMargin + kInnerPad;
        float sy = kFmY + 16.0f;
        float gap = 90.0f;

        // Labels show the routing direction: source > destination
        createKnob(kParamFM3to2, "3>2", 0, 1, 0, NanoKnob::kFormatPercent, fmColor,
                   static_cast<int>(sx), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
        createKnob(kParamFM3to1, "3>1", 0, 1, 0, NanoKnob::kFormatPercent, fmColor,
                   static_cast<int>(sx + gap), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
        createKnob(kParamFM2to1, "2>1", 0, 1, 0, NanoKnob::kFormatPercent, fmColor,
                   static_cast<int>(sx + gap * 2), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
    }

    // ========= FILTER (left half of row 2) =========
    {
        float sx = kMargin + kInnerPad;
        float sy = kFiltY + 18.0f;

        createKnob(kParamFilterCutoff, "Cutoff", 20, 20000, 1200, NanoKnob::kFormatHz, cyan,
                   static_cast<int>(sx), static_cast<int>(sy), kKnobW + 8, kKnobH + 8, kKnobRLarge);
        knobs_[kParamFilterCutoff]->setUsingLogScale(true);

        createKnob(kParamFilterResonance, "Reso", 0, 1, 0.15f, NanoKnob::kFormatPercent, cyan,
                   static_cast<int>(sx + 68), static_cast<int>(sy + 4), kKnobW, kKnobH, kKnobR);
        createKnob(kParamFilterEnvAmount, "Env Amt", -1, 1, 0, NanoKnob::kFormatFloat2, cyan,
                   static_cast<int>(sx + 120), static_cast<int>(sy + 4), kKnobW, kKnobH, kKnobR);
    }

    // ========= AMP ENVELOPE (right half of row 2) =========
    {
        float sx = kFiltColW + kInnerPad;
        float sy = kFiltY + 18.0f;
        float gap = 50.0f;

        createKnob(kParamAmpAttack, "Attack", 0.001f, 5, 0.01f, NanoKnob::kFormatMs, green,
                   static_cast<int>(sx), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
        createKnob(kParamAmpDecay, "Decay", 0.001f, 5, 0.1f, NanoKnob::kFormatMs, green,
                   static_cast<int>(sx + gap), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
        createKnob(kParamAmpSustain, "Sustain", 0, 1, 0.8f, NanoKnob::kFormatPercent, green,
                   static_cast<int>(sx + gap * 2), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
        createKnob(kParamAmpRelease, "Release", 0.001f, 10, 0.15f, NanoKnob::kFormatMs, green,
                   static_cast<int>(sx + gap * 3), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
    }

    // ========= FILTER ENVELOPE (left third of row 3) =========
    {
        float sx = kMargin + kInnerPad;
        float sy = kRow3Y + 18.0f;
        float gap = 50.0f;

        createKnob(kParamFilterAttack, "Attack", 0.001f, 5, 0.01f, NanoKnob::kFormatMs, green,
                   static_cast<int>(sx), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
        createKnob(kParamFilterDecay, "Decay", 0.001f, 5, 0.12f, NanoKnob::kFormatMs, green,
                   static_cast<int>(sx + gap), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
        createKnob(kParamFilterSustain, "Sustain", 0, 1, 0.5f, NanoKnob::kFormatPercent, green,
                   static_cast<int>(sx + gap * 2), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
        createKnob(kParamFilterRelease, "Release", 0.001f, 10, 0.15f, NanoKnob::kFormatMs, green,
                   static_cast<int>(sx + gap * 3), static_cast<int>(sy), kKnobW, kKnobH, kKnobR);
    }

    // ========= LFO (center third of row 3) =========
    {
        float sx = kFltEnvColW + kInnerPad;
        float sy = kRow3Y + 18.0f;

        lfoWaveSel_ = new WaveformSelector(this);
        lfoWaveSel_->setParamIndex(kParamLfoWave);
        lfoWaveSel_->setNumWaveforms(5);
        lfoWaveSel_->setAccentColor(orange);
        lfoWaveSel_->setListener(this);
        lfoWaveSel_->setAbsolutePos(static_cast<int>(sx), static_cast<int>(sy));
        lfoWaveSel_->setSize(static_cast<uint>(kLfoColW - kInnerPad * 2), kWaveSelH);

        float ky = sy + kWaveSelH + 4.0f;
        createKnob(kParamLfoRate, "Rate", 0.1f, 20, 3, NanoKnob::kFormatFloat1, orange,
                   static_cast<int>(sx), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamLfoDepth, "Depth", 0, 1, 0, NanoKnob::kFormatPercent, orange,
                   static_cast<int>(sx + 52), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);
        createKnob(kParamLfoTarget, "Target", 0, 1, 0, NanoKnob::kFormatFloat1, orange,
                   static_cast<int>(sx + 104), static_cast<int>(ky), kKnobW, kKnobH, kKnobR);

        envRetrigger_ = new ToggleSwitch(this);
        envRetrigger_->setParamIndex(kParamEnvRetrigger);
        envRetrigger_->setAccentColor(orange);
        envRetrigger_->setLabel("Retrig");
        envRetrigger_->setListener(this);
        envRetrigger_->setAbsolutePos(static_cast<int>(sx + 156), static_cast<int>(ky + 4));
        envRetrigger_->setSize(48, 34);
        envRetrigger_->setState(true);
    }

    // ========= MASTER + VOICE (right third of row 3) =========
    {
        float sx = kFltEnvColW + kLfoColW + kInnerPad;
        float sy = kRow3Y + 18.0f;

        createKnob(kParamMasterVolume, "Volume", 0, 1, 0.8f, NanoKnob::kFormatPercent, orange,
                   static_cast<int>(sx), static_cast<int>(sy), kKnobW + 8, kKnobH + 8, kKnobRLarge);
        createKnob(kParamDrive, "Drive", 0, 1, 0.15f, NanoKnob::kFormatPercent, orange,
                   static_cast<int>(sx + 64), static_cast<int>(sy + 4), kKnobW, kKnobH, kKnobR);

        // Voice controls in same column
        float vy = sy + 58.0f;

        polyToggle_ = new ToggleSwitch(this);
        polyToggle_->setParamIndex(kParamPolyMode);
        polyToggle_->setAccentColor(cyan);
        polyToggle_->setLabel("Poly");
        polyToggle_->setListener(this);
        polyToggle_->setAbsolutePos(static_cast<int>(sx + 120), static_cast<int>(sy + 4));
        polyToggle_->setSize(42, 30);
        polyToggle_->setState(true);

        createKnob(kParamUnisonCount, "Uni", 1, 7, 1, NanoKnob::kFormatFloat1, cyan,
                   static_cast<int>(sx + 168), static_cast<int>(sy + 4), 40, kKnobH, 13.0f);
        createKnob(kParamUnisonDetune, "Det", 0, 100, 15, NanoKnob::kFormatCents, cyan,
                   static_cast<int>(sx + 212), static_cast<int>(sy + 4), 40, kKnobH, 13.0f);
        createKnob(kParamUnisonSpread, "Spr", 0, 1, 0.5f, NanoKnob::kFormatPercent, cyan,
                   static_cast<int>(sx + 256), static_cast<int>(sy + 4), 40, kKnobH, 13.0f);
    }

    // ========= WAVEFORM VISUALIZER =========
    {
        waveformViz_ = new WaveformVisualizer(this);
        waveformViz_->setAbsolutePos(static_cast<int>(kMargin), static_cast<int>(kVizY));
        waveformViz_->setSize(static_cast<uint>(kW - kMargin * 2), static_cast<uint>(kVizH));
        updateWaveformVisualizer();
    }

    // ========= VIRTUAL KEYBOARD =========
    {
        keyboard_ = new VirtualKeyboard(this);
        keyboard_->setAbsolutePos(static_cast<int>(kMargin), static_cast<int>(kKbdY));
        keyboard_->setSize(static_cast<uint>(kW - kMargin * 2), static_cast<uint>(kKbdH));
        keyboard_->setListener(this);
    }
}

// ---------------------------------------------------------------------------
// Drawing

void VenomEditor::onNanoDisplay()
{
    drawBackground();
    drawHeader();

    const Color purple = oscAccent();
    const Color cyan   = filterAccent();
    const Color green  = envAccent();
    const Color orange = lfoAccent();

    // Section strips (horizontal labels with accent line)
    drawSectionStrip(kMargin, kOscY, kOscColW - kMargin, "OSC 1", purple);
    drawSectionStrip(kOscColW, kOscY, kOscColW, "OSC 2", purple);
    drawSectionStrip(kOscColW * 2, kOscY, kOscColW - kMargin, "OSC 3", purple);

    const Color fmColor = fmAccent();
    drawSectionStrip(kMargin, kFmY, kW - kMargin * 2, "FM MATRIX", fmColor);

    drawSectionStrip(kMargin, kFiltY, kFiltColW - kMargin, "FILTER", cyan);
    drawSectionStrip(kFiltColW, kFiltY, kAmpEnvColW - kMargin, "AMP ENVELOPE", green);

    drawSectionStrip(kMargin, kRow3Y, kFltEnvColW - kMargin, "FILTER ENVELOPE", green);
    drawSectionStrip(kFltEnvColW, kRow3Y, kLfoColW, "LFO", orange);
    drawSectionStrip(kFltEnvColW + kLfoColW, kRow3Y, kMasterColW - kMargin, "MASTER / VOICE", orange);

    // Vertical dividers between columns
    drawVerticalDivider(kOscColW, kOscY + 4.0f, kOscY + kOscH - 4.0f);
    drawVerticalDivider(kOscColW * 2, kOscY + 4.0f, kOscY + kOscH - 4.0f);
    drawVerticalDivider(kFiltColW, kFiltY + 4.0f, kFiltY + kFiltH - 4.0f);
    drawVerticalDivider(kFltEnvColW, kRow3Y + 4.0f, kRow3Y + kRow3H - 4.0f);
    drawVerticalDivider(kFltEnvColW + kLfoColW, kRow3Y + 4.0f, kRow3Y + kRow3H - 4.0f);

    // Horizontal dividers between rows
    beginPath();
    moveTo(kMargin, kFmY);
    lineTo(kW - kMargin, kFmY);
    strokeColor(sectionBorder());
    strokeWidth(1.0f);
    stroke();

    beginPath();
    moveTo(kMargin, kFiltY);
    lineTo(kW - kMargin, kFiltY);
    strokeColor(sectionBorder());
    strokeWidth(1.0f);
    stroke();

    beginPath();
    moveTo(kMargin, kRow3Y);
    lineTo(kW - kMargin, kRow3Y);
    strokeColor(sectionBorder());
    strokeWidth(1.0f);
    stroke();

    beginPath();
    moveTo(kMargin, kRow3Y + kRow3H);
    lineTo(kW - kMargin, kRow3Y + kRow3H);
    strokeColor(sectionBorder());
    strokeWidth(1.0f);
    stroke();

    // Status bar at bottom
    fontSize(8.0f);
    fillColor(textDim());
    textAlign(ALIGN_RIGHT | ALIGN_BOTTOM);
    text(kW - 12.0f, kH - 4.0f, "Poly 8  |  48kHz", nullptr);
}

void VenomEditor::drawBackground()
{
    beginPath();
    rect(0, 0, getWidth(), getHeight());
    fillColor(background());
    fill();
}

void VenomEditor::drawHeader()
{
    beginPath();
    rect(0, 0, getWidth(), kHeaderH);
    fillColor(headerBg());
    fill();

    // Bottom line
    beginPath();
    moveTo(0, kHeaderH);
    lineTo(kW, kHeaderH);
    strokeColor(sectionBorder());
    strokeWidth(1.0f);
    stroke();

    // Brand
    fontSize(15.0f);
    fillColor(oscAccent());
    textAlign(ALIGN_LEFT | ALIGN_MIDDLE);
    text(14.0f, kHeaderH * 0.5f, "V E N O M", nullptr);

    // Version
    fontSize(9.0f);
    fillColor(textDim());
    textAlign(ALIGN_RIGHT | ALIGN_MIDDLE);
    text(getWidth() - 14.0f, kHeaderH * 0.5f, "v0.5.0", nullptr);
}

void VenomEditor::drawSectionStrip(float x, float y, float w,
                                     const char* title, const Color& accent)
{
    // Section background (subtle)
    beginPath();
    rect(x, y, w, 14.0f);
    fillColor(sectionHeaderBg());
    fill();

    // Accent line at top
    beginPath();
    moveTo(x, y);
    lineTo(x + w, y);
    strokeColor(Color(accent.red, accent.green, accent.blue, 0.5f));
    strokeWidth(1.5f);
    stroke();

    // Title text
    fontSize(9.0f);
    fillColor(Color(accent.red, accent.green, accent.blue, 0.7f));
    textAlign(ALIGN_LEFT | ALIGN_MIDDLE);
    text(x + 8.0f, y + 7.0f, title, nullptr);
}

void VenomEditor::drawVerticalDivider(float x, float y1, float y2)
{
    beginPath();
    moveTo(x, y1);
    lineTo(x, y2);
    strokeColor(sectionBorder());
    strokeWidth(1.0f);
    stroke();
}

// ---------------------------------------------------------------------------
// Waveform visualizer sync

void VenomEditor::updateWaveformVisualizer()
{
    if (waveformViz_ == nullptr)
        return;

    waveformViz_->setOscWaveform(0, static_cast<int>(paramValues_[kParamOsc1Wave]));
    waveformViz_->setOscWaveform(1, static_cast<int>(paramValues_[kParamOsc2Wave]));
    waveformViz_->setOscWaveform(2, static_cast<int>(paramValues_[kParamOsc3Wave]));

    waveformViz_->setOscLevel(0, paramValues_[kParamOsc1Level]);
    waveformViz_->setOscLevel(1, paramValues_[kParamOsc2Level]);
    waveformViz_->setOscLevel(2, paramValues_[kParamOsc3Level]);

    waveformViz_->setOscWavePos(0, paramValues_[kParamOsc1WavePos]);
    waveformViz_->setOscWavePos(1, paramValues_[kParamOsc2WavePos]);
    waveformViz_->setOscWavePos(2, paramValues_[kParamOsc3WavePos]);
}

// ---------------------------------------------------------------------------
// Parameter callbacks

void VenomEditor::parameterChanged(uint32_t index, float value)
{
    if (index >= venom::kParameterCount)
        return;

    paramValues_[index] = value;

    if (knobs_[index] != nullptr)
        knobs_[index]->setValue(value, false);

    if (index == kParamOsc1Wave)
        oscWave_[0]->setSelectedWave(static_cast<int>(value));
    else if (index == kParamOsc2Wave)
        oscWave_[1]->setSelectedWave(static_cast<int>(value));
    else if (index == kParamOsc3Wave)
        oscWave_[2]->setSelectedWave(static_cast<int>(value));
    else if (index == kParamEnvRetrigger)
        envRetrigger_->setState(value > 0.5f);
    else if (index == kParamPolyMode)
        polyToggle_->setState(value > 0.5f);
    else if (index == kParamLfoWave)
        lfoWaveSel_->setSelectedWave(static_cast<int>(value));

    if (index == kParamOsc1Wave || index == kParamOsc2Wave || index == kParamOsc3Wave ||
        index == kParamOsc1Level || index == kParamOsc2Level || index == kParamOsc3Level ||
        index == kParamOsc1WavePos || index == kParamOsc2WavePos || index == kParamOsc3WavePos)
    {
        updateWaveformVisualizer();
    }

    repaint();
}

void VenomEditor::stateChanged(const char*, const char*) {}

// ---------------------------------------------------------------------------
// Knob callbacks

void VenomEditor::knobDragStarted(SubWidget* widget)
{
    editParameter(static_cast<NanoKnob*>(widget)->getParamIndex(), true);
}

void VenomEditor::knobDragFinished(SubWidget* widget)
{
    editParameter(static_cast<NanoKnob*>(widget)->getParamIndex(), false);
}

void VenomEditor::knobValueChanged(SubWidget* widget, float value)
{
    setParameterValue(static_cast<NanoKnob*>(widget)->getParamIndex(), value);
}

void VenomEditor::waveformChanged(WaveformSelector* selector, int waveIndex)
{
    uint32_t idx = selector->getParamIndex();
    editParameter(idx, true);
    setParameterValue(idx, static_cast<float>(waveIndex));
    editParameter(idx, false);
}

void VenomEditor::toggleChanged(ToggleSwitch* toggle, bool state)
{
    uint32_t idx = toggle->getParamIndex();
    editParameter(idx, true);
    setParameterValue(idx, state ? 1.0f : 0.0f);
    editParameter(idx, false);
}

// ---------------------------------------------------------------------------
// Preset browser callback

void VenomEditor::presetSelected(PresetBrowser* browser, int index)
{
    if (index < 0 || index >= static_cast<int>(preset::kFactoryPresetCount))
        return;

    const auto& fp = preset::kFactoryPresets[static_cast<size_t>(index)];
    browser->setPresetName(fp.name);

    for (uint32_t i = 0; i < venom::kParameterCount; ++i) {
        editParameter(i, true);
        setParameterValue(i, fp.params[i]);
        editParameter(i, false);
        paramValues_[i] = fp.params[i];
        if (knobs_[i] != nullptr)
            knobs_[i]->setValue(fp.params[i], false);
    }

    oscWave_[0]->setSelectedWave(static_cast<int>(fp.params[kParamOsc1Wave]));
    oscWave_[1]->setSelectedWave(static_cast<int>(fp.params[kParamOsc2Wave]));
    oscWave_[2]->setSelectedWave(static_cast<int>(fp.params[kParamOsc3Wave]));
    envRetrigger_->setState(fp.params[kParamEnvRetrigger] > 0.5f);
    polyToggle_->setState(fp.params[kParamPolyMode] > 0.5f);
    lfoWaveSel_->setSelectedWave(static_cast<int>(fp.params[kParamLfoWave]));

    updateWaveformVisualizer();
    repaint();
}

// ---------------------------------------------------------------------------
// Virtual keyboard callbacks

void VenomEditor::keyPressed(VirtualKeyboard*, uint8_t note, uint8_t velocity)
{
    sendNote(0, note, velocity);
}

void VenomEditor::keyReleased(VirtualKeyboard*, uint8_t note)
{
    sendNote(0, note, 0);
}

// ---------------------------------------------------------------------------
UI* createUI() { return new VenomEditor(); }

END_NAMESPACE_DISTRHO
