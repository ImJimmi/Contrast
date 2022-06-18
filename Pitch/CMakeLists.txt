juce_add_plugin(Pitch
    COMPANY_NAME Contrast
    FORMATS AU VST3 Standalone
    PLUGIN_CODE Pitc
    PLUGIN_MANUFACTURER_CODE Cont
    PRODUCT_NAME "Pitch"
)

target_sources(Pitch
PRIVATE
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
)

target_include_directories(Pitch
PRIVATE
    Source
)

target_compile_definitions(Pitch
PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_features(Pitch
PRIVATE
    cxx_std_20
)

target_link_libraries(Pitch
PRIVATE
    contrast_shared_resources
    juce::juce_audio_utils
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

juce_generate_juce_header(Pitch)
