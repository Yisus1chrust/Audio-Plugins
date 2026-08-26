cmake_minimum_required(VERSION 3.22)

project(PhotoSynthWebView VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PHOTO_SYNTH_FETCH_JUCE "Fetch JUCE automatically if not provided" ON)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
elseif(DEFINED JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
    add_subdirectory(${JUCE_DIR} JUCE)
elseif(PHOTO_SYNTH_FETCH_JUCE)
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 7.0.12
    )
    FetchContent_MakeAvailable(JUCE)
else()
    message(FATAL_ERROR "JUCE not found. Add JUCE as submodule at ./JUCE or pass -DJUCE_DIR=/path/to/JUCE")
endif()

file(GLOB_RECURSE WEB_ASSET_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources/web/*")

juce_add_binary_data(PhotoSynthWebAssets
    NAMESPACE BinaryData
    SOURCES ${WEB_ASSET_FILES}
)

if (UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKITGTK REQUIRED webkit2gtk-4.0 gtk+-x11-3.0)
endif()

juce_add_plugin(PhotoSynthWebView
    COMPANY_NAME "ModularSoundTechnologies"
    BUNDLE_ID com.modularsoundtechnologies.photosynthwebview
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE Mstl
    PLUGIN_CODE Pwvb
    FORMATS VST3 AU
    PRODUCT_NAME "Photo Synth WebView"
)

target_sources(PhotoSynthWebView PRIVATE
    Source/CommonTypes.h
    Source/ImageAnalyzer.h
    Source/ImageAnalyzer.cpp
    Source/MorphEngine.h
    Source/MorphEngine.cpp
    Source/PhysicalModeling.h
    Source/PhysicalModeling.cpp
    Source/EffectsChain.h
    Source/EffectsChain.cpp
    Source/SynthVoice.h
    Source/SynthVoice.cpp
    Source/SynthEngine.h
    Source/SynthEngine.cpp
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
)

target_include_directories(PhotoSynthWebView PRIVATE Source)

if (UNIX AND NOT APPLE)
    target_include_directories(PhotoSynthWebView PRIVATE ${WEBKITGTK_INCLUDE_DIRS})
    target_compile_options(PhotoSynthWebView PRIVATE ${WEBKITGTK_CFLAGS_OTHER})
endif()

target_compile_definitions(PhotoSynthWebView
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(PhotoSynthWebView
    PRIVATE
        PhotoSynthWebAssets
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if (UNIX AND NOT APPLE)
    target_link_libraries(PhotoSynthWebView PRIVATE ${WEBKITGTK_LIBRARIES})
    target_link_directories(PhotoSynthWebView PRIVATE ${WEBKITGTK_LIBRARY_DIRS})
endif()
