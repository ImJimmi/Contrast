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
    Slider thresholdSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;

    Slider ratioSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;

    Slider kneeSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;

    Slider attackSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attackAttachment;

    Slider releaseSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    Slider gainSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
