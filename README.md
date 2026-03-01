# venom

`venom` is a DPF-based synthesizer targeting trap, hyperpop, and hip-hop production.

## Current milestone

This milestone provides a functional, testable synth core and a plugin wrapper wired into it:

- DPF-oriented CMake project structure for VST3 builds.
- Core DSP primitives:
  - Oscillator (sine/saw/square/triangle, detune in cents, pitch in semitones).
  - ADSR envelopes with corrected release behavior and retrigger support.
  - Cascaded 24 dB/oct lowpass filter.
  - Sine LFO (0.1 Hz to 20 Hz).
- `SynthVoice` + `SynthEngine` path:
  - MIDI note-on/off handling.
  - 3-oscillator mixing.
  - Amp/filter envelopes.
  - LFO target routing (pitch/cutoff).
  - Output drive + soft clipping + master volume.
- Plugin wrapper now delegates rendering to `SynthEngine` to keep host and test paths aligned.
- Core tests:
  - `venom_core_smoke`
  - `venom_envelope_release`
  - `venom_midi_note_off`
  - `venom_midi_last_note_priority`
  - `venom_midi_all_notes_off`

## Get DPF (submodule)

DPF is configured as a git submodule at `external/DPF`.

```bash
git submodule update --init --recursive
```

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

> If you keep DPF in another location, you can still override with `-DDPF_PATH=/path/to/DPF`.
