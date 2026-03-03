#pragma once

#include "../VenomShared.hpp"

#include <array>

namespace venom::preset {

struct FactoryPreset {
  const char* name;
  const char* category;
  std::array<float, kParameterCount> params;
};

// Param layout (53 total):
//   [0-1]   Master Vol, Drive
//   [2-5]   Osc1: Wave, Detune, Pitch, Level
//   [6-9]   Osc2: Wave, Detune, Pitch, Level
//   [10-13] Osc3: Wave, Detune, Pitch, Level
//   [14-16] Filter: Cutoff, Resonance, EnvAmount
//   [17-20] Amp Env: A, D, S, R
//   [21-24] Flt Env: A, D, S, R
//   [25-27] LFO: Rate, Depth, Target
//   [28]    Env Retrigger
//   [29-36] Poly, UnisonCount, UnisonDetune, UnisonSpread, WavePos1-3, LfoWave
//   [37-39] FM: 2>1, 3>1, 3>2
//   [40-42] WT Banks: Osc1, Osc2, Osc3
//   [43-52] Granular: Engines1-3, GrainSz, Density, Scatter, PitchRnd, Start, SampleSel, Window

inline const std::array<FactoryPreset, 12> kFactoryPresets = {{
  // 0 — Dark Pad VHS
  {"Dark Pad VHS", "Pad", {{
    0.8f, 0.15f,
    3, -5, 0, 0.45f,
    1, 5, 0, 0.45f,
    0, 0, -12, 0.3f,
    2200, 0.15f, 0,
    0.35f, 0.6f, 0.75f, 1.2f,
    0.01f, 0.12f, 0.5f, 0.15f,
    3, 0.2f, 1,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 1 — Dist Lead Venom
  {"Dist Lead Venom", "Lead", {{
    0.8f, 0.3f,
    2, -5, 0, 0.7f,
    2, 8, 0, 0.55f,
    0, 0, -12, 0.35f,
    3800, 0.28f, 0,
    0.005f, 0.2f, 0.7f, 0.18f,
    0.01f, 0.12f, 0.5f, 0.15f,
    3, 0.3f, 0,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 2 — Hyper Lead Razor
  {"Hyper Lead Razor", "Lead", {{
    0.8f, 0.4f,
    1, -7, 0, 0.6f,
    2, 7, 0, 0.4f,
    1, 0, -12, 0.2f,
    5000, 0.22f, 0.3f,
    0.01f, 0.08f, 0.85f, 0.12f,
    0.01f, 0.12f, 0.5f, 0.15f,
    3, 0.35f, 0,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 3 — Hyper Pluck Glass
  {"Hyper Pluck Glass", "Pluck", {{
    0.8f, 0.15f,
    1, -5, 0, 0.6f,
    3, 5, 0, 0.35f,
    0, 0, -12, 0.35f,
    6000, 0.25f, 0.85f,
    0.001f, 0.09f, 0.1f, 0.05f,
    0.01f, 0.12f, 0.5f, 0.15f,
    3, 0, 0,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 4 — Hyper Pluck Neon
  {"Hyper Pluck Neon", "Pluck", {{
    0.8f, 0.15f,
    2, -5, 0, 0.55f,
    1, 5, 0, 0.55f,
    0, 0, -12, 0.35f,
    4200, 0.35f, 0.75f,
    0.002f, 0.12f, 0.15f, 0.09f,
    0.01f, 0.12f, 0.5f, 0.15f,
    3, 0.25f, 1,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 5 — Trap 808 Earthquake
  {"Trap 808 Earthquake", "Bass", {{
    0.8f, 0.2f,
    1, -5, 0, 0.9f,
    2, 5, 0, 0.25f,
    0, 0, -12, 0.45f,
    900, 0.15f, 0.2f,
    0.01f, 0.08f, 0.95f, 0.12f,
    0.01f, 0.12f, 0.5f, 0.15f,
    3, 0, 0,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 6 — Trap 808 Hollow
  {"Trap 808 Hollow", "Bass", {{
    0.8f, 0.25f,
    0, -5, 0, 0.8f,
    1, 5, 0, 0.3f,
    0, 0, -12, 0.25f,
    700, 0.2f, 0,
    0.01f, 0.08f, 0.85f, 0.12f,
    0.01f, 0.12f, 0.5f, 0.15f,
    3, 0, 0,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 7 — Trap Bell Pluck
  {"Trap Bell Pluck", "Pluck", {{
    0.8f, 0.15f,
    3, -5, 0, 0.6f,
    0, 5, 0, 0.4f,
    0, 0, -12, 0.35f,
    5200, 0.3f, 0.6f,
    0.01f, 0.15f, 0.25f, 0.12f,
    0.01f, 0.12f, 0.5f, 0.15f,
    3, 0, 0,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 8 — Trap Sub Glide
  {"Trap Sub Glide", "Bass", {{
    0.8f, 0.2f,
    0, -5, 0, 0.6f,
    0, 5, 0, 0.2f,
    0, 0, -12, 0.5f,
    1200, 0.15f, 0,
    0.01f, 0.08f, 0.85f, 0.12f,
    0.01f, 0.12f, 0.5f, 0.15f,
    3, 0, 0,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 9 — Wide Pad Ether
  {"Wide Pad Ether", "Pad", {{
    0.8f, 0.15f,
    1, -5, 0, 0.6f,
    1, 11, 0, 0.4f,
    3, 0, -12, 0.25f,
    2600, 0.15f, 0,
    0.28f, 0.55f, 0.8f, 1.4f,
    0.01f, 0.12f, 0.5f, 0.15f,
    3, 0.18f, 0,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 10 — FM Bell  (Phase 1 showcase)
  // Sine carriers, Osc3 modulates Osc1 for bell harmonics
  {"FM Bell", "Bell", {{
    0.8f, 0.05f,
    0, 0, 0, 0.7f,       // Osc1: Sine, no detune, root pitch
    0, 0, 7, 0.0f,       // Osc2: Sine, +7 semi (perfect 5th), silent in mix
    0, 0, 19, 0.0f,      // Osc3: Sine, +19 semi (~ratio 3:1), silent in mix
    8000, 0.1f, 0.6f,
    0.001f, 0.4f, 0.0f, 0.8f,   // Amp: instant attack, long decay, no sustain
    0.001f, 0.3f, 0.2f, 0.5f,   // Flt: fast attack/decay, low sustain
    3, 0, 0,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0, 0.45f, 0,         // FM: no 2>1, FM3>1=0.45, no 3>2
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},

  // 11 — FM Bass Growl  (Phase 1 showcase)
  // Saw carrier, Osc2→Osc1 for gritty FM bass
  {"FM Bass Growl", "Bass", {{
    0.8f, 0.3f,
    1, -5, 0, 0.8f,      // Osc1: Saw, slight detune, root
    0, 0, 0, 0.15f,      // Osc2: Sine, root pitch, quiet in mix
    0, 0, -12, 0.3f,     // Osc3: Sine, sub octave
    1800, 0.35f, 0.4f,
    0.005f, 0.15f, 0.8f, 0.1f,  // Amp: fast attack, short decay, high sustain
    0.005f, 0.2f, 0.4f, 0.15f,  // Flt: moderate envelope
    3, 0, 0,
    1,
    1, 1, 15, 0.5f,
    0, 0, 0, 0,
    0.55f, 0, 0.3f,      // FM: 2>1=0.55, no 3>1, 3>2=0.3
    0, 0, 0,
    0, 0, 0, 80, 8, 0.1f, 0, 0, 0, 0
  }}},
}};

inline constexpr size_t kFactoryPresetCount = kFactoryPresets.size();

} // namespace venom::preset
