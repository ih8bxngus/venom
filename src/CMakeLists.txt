set(VENOM_PLUGIN_SOURCES
  VenomPlugin.cpp
  VenomEditor.cpp
)

set(VENOM_DSP_HEADERS
  VenomShared.hpp
  core/SynthEngine.hpp
  preset/PresetManager.hpp
  dsp/Oscillator.hpp
  dsp/EnvelopeADSR.hpp
  dsp/Lowpass24.hpp
  dsp/LFO.hpp
  dsp/Wavetable.hpp
  dsp/UnisonOscBank.hpp
  dsp/SynthVoice.hpp
)

add_library(venom_dsp INTERFACE)
target_include_directories(venom_dsp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(venom_dsp INTERFACE ${VENOM_DSP_HEADERS})

if(COMMAND dpf_add_plugin)
  dpf_add_plugin(venom
    TARGETS vst3
    UI_TYPE opengl
    FILES_DSP
      VenomPlugin.cpp
    FILES_UI
      VenomEditor.cpp
      ui/NanoKnob.cpp
      ui/WaveformSelector.cpp
      ui/ToggleSwitch.cpp
      ui/PresetBrowser.cpp
      ui/WaveformVisualizer.cpp
      ui/VirtualKeyboard.cpp
  )

  target_include_directories(venom PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

  target_compile_definitions(venom PUBLIC
    DISTRHO_PLUGIN_BRAND="venom"
    DISTRHO_PLUGIN_NAME="venom"
    DISTRHO_PLUGIN_URI="https://github.com/ih8bxngus/venom"
  )
endif()
