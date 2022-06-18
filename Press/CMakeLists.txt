juce_add_plugin(Press
    COMPANY_NAME Contrast
    FORMATS AU VST3 Standalone
    PLUGIN_CODE Pres
    PLUGIN_MANUFACTURER_CODE Cont
    PRODUCT_NAME "Press"
)

target_sources(Press
PRIVATE
    Source/Audio/PressProcessor.cpp
    Source/Audio/PressProcessor.h
    Source/GUI/PressEditor.cpp
    Source/GUI/PressEditor.h
    Source/Press.h
)

target_include_directories(Press
PRIVATE
    Source
)

target_compile_definitions(Press
PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_features(Press
PRIVATE
    cxx_std_20
)

target_compile_options(Press
PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)


target_link_libraries(Press
PRIVATE
    contrast_shared_resources
    juce::juce_audio_utils
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

juce_generate_juce_header(Press)
