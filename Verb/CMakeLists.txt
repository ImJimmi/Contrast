juce_add_plugin(Verb
    COMPANY_NAME Contrast
    FORMATS AU VST3 Standalone
    PLUGIN_CODE Verb
    PLUGIN_MANUFACTURER_CODE Cont
    PRODUCT_NAME "Verb"
)

target_sources(Verb
PRIVATE
    Source/Audio/VerbProcessor.cpp
    Source/Audio/VerbProcessor.h
    Source/GUI/VerbEditor.cpp
    Source/GUI/VerbEditor.h
    Source/Verb.h
)

target_include_directories(Verb
PRIVATE
    Source
)

target_compile_definitions(Verb
PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_features(Verb
PRIVATE
    cxx_std_20
)

target_link_libraries(Verb
PRIVATE
    contrast_shared_resources
    juce::juce_audio_utils
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

juce_generate_juce_header(Verb)
