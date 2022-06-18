juce_add_plugin(Gate
    COMPANY_NAME Contrast
    FORMATS AU VST3 Standalone
    PLUGIN_CODE Gate
    PLUGIN_MANUFACTURER_CODE Cont
    PRODUCT_NAME "Gate"
)

target_sources(Gate
PRIVATE
    Source/Audio/GateProcessor.cpp
    Source/Audio/GateProcessor.h
    Source/GUI/GateEditor.cpp
    Source/GUI/GateEditor.h
    Source/Gate.h
)

target_include_directories(Gate
PRIVATE
    Source
)

target_compile_definitions(Gate
PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_features(Gate
PRIVATE
    cxx_std_20
)

target_link_libraries(Gate
PRIVATE
    contrast_shared_resources
    juce::juce_audio_utils
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

juce_generate_juce_header(Gate)
