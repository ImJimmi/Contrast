#pragma once

#include <JuceHeader.h>

//======================================================================================================================
class PressProcessor    :   public contrast::PluginProcessor
{
public:
    //==================================================================================================================
    PressProcessor();
    ~PressProcessor();

    //==================================================================================================================
    void prepareToPlay(double, int) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;

    void numChannelsChanged() override;

    //==================================================================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==================================================================================================================
    juce::StringArray getPresetNames() const override;

private:
    //==================================================================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() const override;
    juce::ValueTree createDefaultProperties() const override;
    void presetChoiceChanged(int) override;

    void updateCompressors();

    //==================================================================================================================
    // Need a Compressor object for each channel.
    std::vector<std::unique_ptr<contrast::Compressor>> compressors;

    // Parameter references for easy access.
    juce::AudioParameterFloat& threshold;
    juce::AudioParameterFloat& ratio;
    juce::AudioParameterFloat& knee;
    juce::AudioParameterFloat& attack;
    juce::AudioParameterFloat& release;
    juce::AudioParameterFloat& gain;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PressProcessor)
};
