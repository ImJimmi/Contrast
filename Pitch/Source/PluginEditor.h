#pragma once

#include "JuceHeader.h"
#include "PluginProcessor.h"

//======================================================================================================================
class PluginEditor  :   public AudioProcessorEditor
{
public:
    //==================================================================================================================
    PluginEditor(PluginProcessor&);
    ~PluginEditor();

    //==================================================================================================================
    void paint(Graphics&) override;
    void resized() override;

private:
    //==================================================================================================================
    PluginProcessor& processor;

    // The custom LookAndFeel class we'll be using for this plugin.
    contrast::LookAndFeel contrastLaF;

    // The header component, displayed across the top of the UI.
    contrast::HeaderComponent header;

    // Sliders and their corresponding attachments.
    Slider semitonesSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> semitonesAttachment;

    Slider centsSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> centsAttachment;

    Slider mixSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
