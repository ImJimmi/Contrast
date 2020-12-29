/***********************************************************************************************************************
BEGIN_JUCE_MODULE_DECLARATION

    ID:                 contrast_shared_resources
    vendor:             James Johnson
    version:            1.0.0
    name:               Contrast Shared Resources
    description:        Classes used across several Contrast plugins.
    website:            https://github.com/ImJimmi
    license:            GNU General Public License

    dependencies:       juce_audio_basics, juce_audio_processors, juce_core,
                        juce_gui_basics

END_JUCE_MODULE_DECLARATION
***********************************************************************************************************************/

#pragma once
#define CONTRAST_SHARED_RESOURCES_H_INCLUDED

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    // Forward declarations.
    class HeaderComponent;

    //==================================================================================================================
    // Constants.

    // The width of small sliders.
    template <typename T>
    constexpr T sliderWidthSmall = static_cast<T>(65);

    // The width of large sliders.
    template <typename T>
    constexpr T sliderWidthLarge = static_cast<T>(80);

    // The size of the gap between widgets.
    template <typename T>
    constexpr T widgetGap = static_cast<T>(20);

    // The default thickness of lines/borders.
    // This value is roughly based on the thickness of Glyphs in the Arial
    // Black font used by the LookAndFeel class.
    template <typename T>
    constexpr T defaultThickness = static_cast<T>(3);

    /** Common properties used by multiple plugins. */
    namespace PropertyIDs
    {
        constexpr char USE_WHITE_AS_PRIMARY_COLOUR[] = "useWhiteAsPrimaryColour";
        constexpr char PRESET_INDEX[]                = "presetIndex";
    }   // namespace PropertyIds
}   // namespace contrast

//======================================================================================================================
// JUCE includes.
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

//======================================================================================================================
// Contrast includes.
#include "utilities/contrast_functions.h"
#include "utilities/contrast_PluginProcessor.h"

#include "audio/contrast_EnvelopeFollower.h"
#include "audio/contrast_Compressor.h"
#include "audio/contrast_DelayLine.h"

#include "graphics/contrast_LookAndFeel.h"
#include "graphics/icons/contrast_Icons.h"
#include "graphics/components/contrast_HeaderComponent.h"
