/***********************************************************************************************************************
BEGIN_JUCE_MODULE_DECLARATION

    ID:                 contrast_shared_resources
    vendor:             James Johnson
    version:            1.0.0
    name:               Contrast Shared Resources
    description:        Classes used across several Contrast plugins.
    website:            https://github.com/ImJimmi
    license:            GNU General Public License

    dependencies:       juce_core, juce_gui_basics

END_JUCE_MODULE_DECLARATION
***********************************************************************************************************************/

#pragma once
#define CONTRAST_SHARED_RESOURCES_H_INCLUDED

namespace contrast
{
    class HeaderComponent;
}

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "utilities/contrast_functions.h"

#include "audio/contrast_EnvelopeFollower.h"

#include "graphics/contrast_LookAndFeel.h"
#include "graphics/components/contrast_HeaderComponent.h"
