cmake_minimum_required(VERSION 3.15)

project(contrast VERSION 1.2.0)

# ==============================================================================
set(JUCE_PATH $ENV{HOME}/JUCE CACHE FILEPATH "Path to top-level JUCE directory.")
add_subdirectory(${JUCE_PATH} ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

# ==============================================================================
juce_add_module(contrast_shared_resources)
add_subdirectory(Gate)
add_subdirectory(Pitch)
add_subdirectory(Press)
add_subdirectory(Verb)
