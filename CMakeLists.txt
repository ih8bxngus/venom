cmake_minimum_required(VERSION 3.18)
project(venom VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

option(VENOM_BUILD_VST3 "Build VST3 target" ON)
option(VENOM_BUILD_TESTS "Build core smoke tests" ON)

if(NOT DEFINED DPF_PATH)
  set(DPF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/DPF")
endif()

if(EXISTS "${DPF_PATH}/CMakeLists.txt")
  add_subdirectory("${DPF_PATH}" "${CMAKE_BINARY_DIR}/dpf")
else()
  message(WARNING "DPF not found at ${DPF_PATH}. Run `git submodule update --init --recursive` or configure with -DDPF_PATH=<path-to-DPF>.")
endif()

add_subdirectory(src)

if(VENOM_BUILD_TESTS)
  enable_testing()

  add_executable(venom_core_smoke tests/smoke_core.cpp)
  target_link_libraries(venom_core_smoke PRIVATE venom_dsp)
  add_test(NAME venom_core_smoke COMMAND venom_core_smoke)

  add_executable(venom_envelope_release tests/envelope_release.cpp)
  target_link_libraries(venom_envelope_release PRIVATE venom_dsp)
  add_test(NAME venom_envelope_release COMMAND venom_envelope_release)

  add_executable(venom_midi_note_off tests/midi_note_off.cpp)
  target_link_libraries(venom_midi_note_off PRIVATE venom_dsp)
  add_test(NAME venom_midi_note_off COMMAND venom_midi_note_off)

  add_executable(venom_midi_last_note_priority tests/midi_last_note_priority.cpp)
  target_link_libraries(venom_midi_last_note_priority PRIVATE venom_dsp)
  add_test(NAME venom_midi_last_note_priority COMMAND venom_midi_last_note_priority)

  add_executable(venom_midi_all_notes_off tests/midi_all_notes_off.cpp)
  target_link_libraries(venom_midi_all_notes_off PRIVATE venom_dsp)
  add_test(NAME venom_midi_all_notes_off COMMAND venom_midi_all_notes_off)

  add_executable(venom_preset_roundtrip tests/preset_roundtrip.cpp)
  target_link_libraries(venom_preset_roundtrip PRIVATE venom_dsp)
  add_test(NAME venom_preset_roundtrip COMMAND venom_preset_roundtrip)

  add_executable(venom_preset_factory_bank tests/preset_factory_bank.cpp)
  target_link_libraries(venom_preset_factory_bank PRIVATE venom_dsp)
  add_test(NAME venom_preset_factory_bank COMMAND venom_preset_factory_bank)

  add_executable(venom_preset_invalid_json tests/preset_invalid_json.cpp)
  target_link_libraries(venom_preset_invalid_json PRIVATE venom_dsp)
  add_test(NAME venom_preset_invalid_json COMMAND venom_preset_invalid_json)

  add_executable(venom_polyphony_basic tests/polyphony_basic.cpp)
  target_link_libraries(venom_polyphony_basic PRIVATE venom_dsp)
  add_test(NAME venom_polyphony_basic COMMAND venom_polyphony_basic)

  add_executable(venom_unison_stereo tests/unison_stereo.cpp)
  target_link_libraries(venom_unison_stereo PRIVATE venom_dsp)
  add_test(NAME venom_unison_stereo COMMAND venom_unison_stereo)

  add_executable(venom_wavetable_basic tests/wavetable_basic.cpp)
  target_link_libraries(venom_wavetable_basic PRIVATE venom_dsp)
  add_test(NAME venom_wavetable_basic COMMAND venom_wavetable_basic)

  add_executable(venom_preset_migration tests/preset_migration.cpp)
  target_link_libraries(venom_preset_migration PRIVATE venom_dsp)
  add_test(NAME venom_preset_migration COMMAND venom_preset_migration)

  # v0.5 FM synthesis tests
  add_executable(venom_fm_basic tests/fm_basic.cpp)
  target_link_libraries(venom_fm_basic PRIVATE venom_dsp)
  add_test(NAME venom_fm_basic COMMAND venom_fm_basic)

  add_executable(venom_fm_zero_passthrough tests/fm_zero_passthrough.cpp)
  target_link_libraries(venom_fm_zero_passthrough PRIVATE venom_dsp)
  add_test(NAME venom_fm_zero_passthrough COMMAND venom_fm_zero_passthrough)

  add_executable(venom_fm_render_order tests/fm_render_order.cpp)
  target_link_libraries(venom_fm_render_order PRIVATE venom_dsp)
  add_test(NAME venom_fm_render_order COMMAND venom_fm_render_order)
endif()
