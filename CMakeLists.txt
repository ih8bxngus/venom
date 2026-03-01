cmake_minimum_required(VERSION 3.18)
project(venom VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

option(VENOM_BUILD_VST3 "Build VST3 target" ON)
option(VENOM_BUILD_TESTS "Build core smoke tests" ON)

if(NOT DEFINED DPF_PATH)
  set(DPF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/DPF")
endif()

if(EXISTS "${DPF_PATH}/cmake/DPF.cmake")
  include("${DPF_PATH}/cmake/DPF.cmake")
else()
  message(WARNING "DPF.cmake not found at ${DPF_PATH}. Run `git submodule update --init --recursive` or configure with -DDPF_PATH=<path-to-DPF>.")
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
endif()
