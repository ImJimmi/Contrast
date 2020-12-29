#pragma once

#include "JuceHeader.h"
#include "../Audio/VerbProcessor.h"

//======================================================================================================================
class VerbEditor    :   public juce::AudioProcessorEditor
{
public:
    //==================================================================================================================
    VerbEditor(VerbProcessor&);
    ~VerbEditor();

    //==================================================================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //==================================================================================================================
    VerbProcessor& processor;

    // The custom LookAndFeel class we'll be using for this plugin.
    contrast::LookAndFeel contrastLaF;

    // The header component, displayed across the top of the UI.
    contrast::HeaderComponent header;

    juce::Slider sizeSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttachment;

    juce::Slider dampingSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;

    juce::Slider wetSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetAttachment;

    juce::Slider drySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryAttachment;

    juce::Slider widthSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerbEditor)
};
