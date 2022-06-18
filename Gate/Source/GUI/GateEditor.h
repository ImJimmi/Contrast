#pragma once

#include <JuceHeader.h>

#include "../Audio/GateProcessor.h"

//======================================================================================================================
class GateEditor    :   public juce::AudioProcessorEditor
{
public:
    //==================================================================================================================
    GateEditor(GateProcessor&);
    ~GateEditor() final;

    //==================================================================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //==================================================================================================================
    GateProcessor& gateProcessor;

    // The custom LookAndFeel class we'll be using for this plugin.
    contrast::LookAndFeel contrastLaF;

    // The header component, displayed across the top of the UI.
    contrast::HeaderComponent header;

    juce::Slider thresholdSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;

    juce::Slider attackSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;

    juce::Slider releaseSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GateEditor)
};
