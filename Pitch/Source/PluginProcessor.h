#pragma once

#include <JuceHeader.h>

//#include "../Pitch.h"

//======================================================================================================================
class PluginProcessor   :   public contrast::PluginProcessor,
                            private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==================================================================================================================
    PluginProcessor();
    ~PluginProcessor() final;

    //==================================================================================================================
    void prepareToPlay(double, int) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;

    void numChannelsChanged() override;

    //==================================================================================================================
    AudioProcessorEditor* createEditor() override;

    //==================================================================================================================
    juce::StringArray getPresetNames() const override;

private:
    //==================================================================================================================
    /** Creates the audio parameters for our plugin which are passed to the
        APVTS when it is created.
    */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() const override;

    juce::ValueTree createDefaultProperties() const override;

    void parameterChanged(const String&, float) override;
    void presetChoiceChanged(int newPresetIndex) override;

    //==================================================================================================================
    // Need a Compressor object for each channel.
    std::vector<std::unique_ptr<contrast::PitchShifter>> pitShifters;

    // Parameter references for easy access.
    AudioParameterInt& semitones;
    AudioParameterFloat& cents;
    AudioParameterFloat& mix;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
