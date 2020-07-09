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

    Slider sizeSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> sizeAttachment;

    Slider dampingSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;

    Slider wetSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> wetAttachment;

    Slider drySlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> dryAttachment;

    Slider widthSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> widthAttachment;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
