#pragma once

#include "JuceHeader.h"
#include "../Audio/PressProcessor.h"

//======================================================================================================================
class PressEditor   :   public juce::AudioProcessorEditor
{
public:
    //==================================================================================================================
    PressEditor(PressProcessor&);
    ~PressEditor();

    //==================================================================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //==================================================================================================================
    PressProcessor& processor;

    // The custom LookAndFeel class we'll be using for this plugin.
    contrast::LookAndFeel contrastLaF;

    // The header component, displayed across the top of the UI.
    contrast::HeaderComponent header;

    // Sliders and their corresponding attachments.
    juce::Slider thresholdSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;

    juce::Slider ratioSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;

    juce::Slider kneeSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;

    juce::Slider attackSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;

    juce::Slider releaseSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    juce::Slider gainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PressEditor)
};
