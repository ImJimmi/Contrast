#pragma once

#include "JuceHeader.h"

//======================================================================================================================
class VerbProcessor   :   public contrast::PluginProcessor
{
public:
    //==================================================================================================================
    VerbProcessor();
    ~VerbProcessor() final;

    //==================================================================================================================
    void prepareToPlay(double, int) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout&) const override;

    //==================================================================================================================
    juce::AudioProcessorEditor* createEditor() override;

    //==================================================================================================================
    juce::StringArray getPresetNames() const override;

private:
    //==================================================================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() const override;
    juce::ValueTree createDefaultProperties() const override;
    void presetChoiceChanged(int) override;

    //==================================================================================================================
    // The Reverb effecct this plugin will use.
    // It handles stereo so we only need the one.
    juce::Reverb reverb;

    // Hold references to the parameters for easy access.
    juce::AudioParameterFloat& size;
    juce::AudioParameterFloat& damping;
    juce::AudioParameterFloat& wet;
    juce::AudioParameterFloat& dry;
    juce::AudioParameterFloat& width;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerbProcessor)
};
