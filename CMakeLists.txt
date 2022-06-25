cmake_minimum_required(VERSION 3.15)

project(contrast VERSION 1.2.0)

option(DUMP_VERSION "Whether or not to dump the project version number to a VERSION.txt file." OFF)
if (DUMP_VERSION)
    file(WRITE VERSION.txt ${CMAKE_PROJECT_VERSION})
endif()

# ==============================================================================
add_subdirectory(JUCE)

# ==============================================================================
juce_add_module(contrast_shared_resources)
add_subdirectory(Gate)
add_subdirectory(Pitch)
add_subdirectory(Press)
add_subdirectory(Verb)
