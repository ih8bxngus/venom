#include "VenomPlugin.hpp"

#include <algorithm>

START_NAMESPACE_DISTRHO

namespace {

DISTRHO::ParameterRanges makeRanges(float def, float min, float max) {
  DISTRHO::ParameterRanges ranges;
  ranges.def = def;
  ranges.min = min;
  ranges.max = max;
  return ranges;
}

const char* parameterName(uint32_t index) {
  switch (index) {
    case venom::kParamMasterVolume: return "Master Volume";
    case venom::kParamDrive: return "Drive";
    case venom::kParamOsc1Wave: return "Osc1 Wave";
    case venom::kParamOsc1Detune: return "Osc1 Detune";
    case venom::kParamOsc1Pitch: return "Osc1 Pitch";
    case venom::kParamOsc1Level: return "Osc1 Level";
    case venom::kParamOsc2Wave: return "Osc2 Wave";
    case venom::kParamOsc2Detune: return "Osc2 Detune";
    case venom::kParamOsc2Pitch: return "Osc2 Pitch";
    case venom::kParamOsc2Level: return "Osc2 Level";
    case venom::kParamOsc3Wave: return "Osc3 Wave";
    case venom::kParamOsc3Detune: return "Osc3 Detune";
    case venom::kParamOsc3Pitch: return "Osc3 Pitch";
    case venom::kParamOsc3Level: return "Osc3 Level";
    case venom::kParamFilterCutoff: return "Filter Cutoff";
    case venom::kParamFilterResonance: return "Filter Resonance";
    case venom::kParamFilterEnvAmount: return "Filter Env Amount";
    case venom::kParamAmpAttack: return "Amp Attack";
    case venom::kParamAmpDecay: return "Amp Decay";
    case venom::kParamAmpSustain: return "Amp Sustain";
    case venom::kParamAmpRelease: return "Amp Release";
    case venom::kParamFilterAttack: return "Filter Attack";
    case venom::kParamFilterDecay: return "Filter Decay";
    case venom::kParamFilterSustain: return "Filter Sustain";
    case venom::kParamFilterRelease: return "Filter Release";
    case venom::kParamLfoRate: return "LFO Rate";
    case venom::kParamLfoDepth: return "LFO Depth";
    case venom::kParamLfoTarget: return "LFO Target";
    case venom::kParamEnvRetrigger: return "Env Retrigger";
    default: return "Param";
  }
}

} // namespace

VenomPlugin::VenomPlugin()
    : Plugin(venom::kParameterCount, 0, 0) {
  params_.fill(0.0f);

  params_[venom::kParamMasterVolume] = 0.8f;
  params_[venom::kParamDrive] = 0.15f;

  params_[venom::kParamOsc1Wave] = static_cast<float>(venom::Waveform::Saw);
  params_[venom::kParamOsc1Detune] = -5.0f;
  params_[venom::kParamOsc1Pitch] = 0.0f;
  params_[venom::kParamOsc1Level] = 0.6f;

  params_[venom::kParamOsc2Wave] = static_cast<float>(venom::Waveform::Square);
  params_[venom::kParamOsc2Detune] = 5.0f;
  params_[venom::kParamOsc2Pitch] = 0.0f;
  params_[venom::kParamOsc2Level] = 0.4f;

  params_[venom::kParamOsc3Wave] = static_cast<float>(venom::Waveform::Sine);
  params_[venom::kParamOsc3Detune] = 0.0f;
  params_[venom::kParamOsc3Pitch] = -12.0f;
  params_[venom::kParamOsc3Level] = 0.35f;

  params_[venom::kParamFilterCutoff] = 1200.0f;
  params_[venom::kParamFilterResonance] = 0.15f;
  params_[venom::kParamFilterEnvAmount] = 0.0f;

  params_[venom::kParamAmpAttack] = 0.01f;
  params_[venom::kParamAmpDecay] = 0.08f;
  params_[venom::kParamAmpSustain] = 0.85f;
  params_[venom::kParamAmpRelease] = 0.12f;

  params_[venom::kParamFilterAttack] = 0.01f;
  params_[venom::kParamFilterDecay] = 0.12f;
  params_[venom::kParamFilterSustain] = 0.5f;
  params_[venom::kParamFilterRelease] = 0.15f;

  params_[venom::kParamLfoRate] = 3.0f;
  params_[venom::kParamLfoDepth] = 0.0f;
  params_[venom::kParamLfoTarget] = 0.0f;

  params_[venom::kParamEnvRetrigger] = 1.0f;

  syncParameters();
}

const char* VenomPlugin::getLabel() const noexcept { return "venom"; }
const char* VenomPlugin::getDescription() const { return "3-oscillator trap/hyperpop synth built with DPF"; }
const char* VenomPlugin::getMaker() const noexcept { return "venom"; }
const char* VenomPlugin::getHomePage() const noexcept { return "https://github.com/example/venom"; }
const char* VenomPlugin::getLicense() const noexcept { return "MIT"; }
uint32_t VenomPlugin::getVersion() const noexcept { return d_version(0, 3, 0); }
int64_t VenomPlugin::getUniqueId() const noexcept { return d_cconst('v', 'n', 'm', '1'); }

void VenomPlugin::initParameter(uint32_t index, Parameter& parameter) {
  parameter.hints = kParameterIsAutomatable;
  parameter.name = parameterName(index);

  switch (index) {
    case venom::kParamMasterVolume:
      parameter.ranges = makeRanges(0.8f, 0.0f, 1.0f);
      break;
    case venom::kParamDrive:
      parameter.ranges = makeRanges(0.15f, 0.0f, 1.0f);
      break;

    case venom::kParamOsc1Wave:
    case venom::kParamOsc2Wave:
    case venom::kParamOsc3Wave:
      parameter.ranges = makeRanges(1.0f, 0.0f, 3.0f);
      break;
    case venom::kParamOsc1Detune:
    case venom::kParamOsc2Detune:
    case venom::kParamOsc3Detune:
      parameter.ranges = makeRanges(0.0f, -50.0f, 50.0f);
      break;
    case venom::kParamOsc1Pitch:
    case venom::kParamOsc2Pitch:
    case venom::kParamOsc3Pitch:
      parameter.ranges = makeRanges(0.0f, -12.0f, 12.0f);
      break;
    case venom::kParamOsc1Level:
    case venom::kParamOsc2Level:
    case venom::kParamOsc3Level:
      parameter.ranges = makeRanges(0.5f, 0.0f, 1.0f);
      break;

    case venom::kParamFilterCutoff:
      parameter.ranges = makeRanges(1200.0f, 20.0f, 20000.0f);
      break;
    case venom::kParamFilterResonance:
      parameter.ranges = makeRanges(0.15f, 0.0f, 1.0f);
      break;
    case venom::kParamFilterEnvAmount:
      parameter.ranges = makeRanges(0.0f, -1.0f, 1.0f);
      break;

    case venom::kParamAmpAttack:
    case venom::kParamFilterAttack:
      parameter.ranges = makeRanges(0.01f, 0.001f, 5.0f);
      break;
    case venom::kParamAmpDecay:
    case venom::kParamFilterDecay:
      parameter.ranges = makeRanges(0.1f, 0.001f, 5.0f);
      break;
    case venom::kParamAmpSustain:
    case venom::kParamFilterSustain:
      parameter.ranges = makeRanges(0.8f, 0.0f, 1.0f);
      break;
    case venom::kParamAmpRelease:
    case venom::kParamFilterRelease:
      parameter.ranges = makeRanges(0.15f, 0.001f, 10.0f);
      break;

    case venom::kParamLfoRate:
      parameter.ranges = makeRanges(3.0f, 0.1f, 20.0f);
      break;
    case venom::kParamLfoDepth:
      parameter.ranges = makeRanges(0.0f, 0.0f, 1.0f);
      break;
    case venom::kParamLfoTarget:
      parameter.ranges = makeRanges(0.0f, 0.0f, 1.0f);
      break;

    case venom::kParamEnvRetrigger:
      parameter.hints = kParameterIsAutomatable | kParameterIsBoolean;
      parameter.ranges = makeRanges(1.0f, 0.0f, 1.0f);
      break;

    default:
      parameter.ranges = makeRanges(0.0f, 0.0f, 1.0f);
      break;
  }
}

float VenomPlugin::getParameterValue(uint32_t index) const {
  return index < params_.size() ? params_[index] : 0.0f;
}

void VenomPlugin::setParameterValue(uint32_t index, float value) {
  if (index >= params_.size())
    return;

  params_[index] = value;
  syncParameters();
}

void VenomPlugin::activate() {
  engine_.setSampleRate(getSampleRate());
  syncParameters();
}

void VenomPlugin::run(const float**, float** outputs, uint32_t frames, const MidiEvent* midiEvents, uint32_t midiEventCount) {
  if (outputs == nullptr)
    return;

  float* left = outputs[0];
  float* right = outputs[1] != nullptr ? outputs[1] : outputs[0];

  midiScratch_.clear();
  midiScratch_.reserve(midiEventCount);

  for (uint32_t i = 0; i < midiEventCount; ++i) {
    const MidiEvent& in = midiEvents[i];
    if (in.size < 3)
      continue;

    venom::core::MidiEvent out;
    out.frame = in.frame;
    out.status = in.data[0];
    out.data1 = in.data[1];
    out.data2 = in.data[2];
    midiScratch_.push_back(out);
  }

  const venom::core::MidiEvent* midiPtr = midiScratch_.empty() ? nullptr : midiScratch_.data();
  const uint32_t midiCount = static_cast<uint32_t>(midiScratch_.size());
  engine_.process(midiPtr, midiCount, left, right, frames);
}

void VenomPlugin::syncParameters() {
  engine_.setParameters(params_);
}

Plugin* createPlugin() {
  return new VenomPlugin();
}

END_NAMESPACE_DISTRHO
