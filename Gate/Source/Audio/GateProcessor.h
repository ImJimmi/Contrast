#pragma once

#include <JuceHeader.h>

#include "../Gate.h"

//======================================================================================================================
class GateProcessor :   public contrast::PluginProcessor,
                        private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==================================================================================================================
    GateProcessor();
    ~GateProcessor();

    //==================================================================================================================
    void prepareToPlay(double, int) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;

    void numChannelsChanged() override;

    //==================================================================================================================
    juce::AudioProcessorEditor* createEditor() override;

    //==================================================================================================================
    juce::StringArray getPresetNames() const override;

private:
    //==================================================================================================================
    /** Creates the audio parameters for our plugin which are passed to the
        APVTS when it is created.
    */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() const override;

    /** Creates the default properties for the plugin.
        These are any non-audio properties we want to be stored along with the
        plugin's state.
    */
    juce::ValueTree createDefaultProperties() const override;

    void parameterChanged(const juce::String&, float) override;
    void presetChoiceChanged(int newPresetIndex) override;

    //==================================================================================================================
    /** Called by processBlock and processBlockBypassed.

        This ensures the same latency is reported to the host and that the gate
        states are updated even when the plugin is bypassed.
    */
    void process(juce::AudioBuffer<float>&, bool isBypassed);

    /** Updates the length of the delay lines based on the current attack. */
    void updateDelayLines();

    //==================================================================================================================
    // The parameters are stored in the APVTS so we'll hold references to them
    // so we can access their values easily.
    juce::AudioParameterFloat& threshold;
    juce::AudioParameterFloat& attack;
    juce::AudioParameterFloat& release;

    // Two envelopes to follow the current and delayed peaks. This is so we can
    // know when to first open the gate (using the current envelope) and then
    // when to close the gate (using the delayed envelope).
    // We need one follower for each channel, so they're declared as vectors
    // (but don't worry, we won't be allocating on the audio thread!).
    std::vector<std::unique_ptr<contrast::EnvelopeFollower>> currentPeakFollowers;
    std::vector<std::unique_ptr<contrast::EnvelopeFollower>> delayedPeakFollowers;

    // The actual gates which use a linearly smoothed value to allow the gate
    // to gradually open and close, instead of instantly stepping from 0 gain to
    // +1 gain.
    std::vector<juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>> gates;

    // The delay lines that allow us to use a look-ahead technique.
    std::vector<std::unique_ptr<contrast::DelayLine<float>>> delayLines;

    // The amount of latency, in samples, that our plugin is introducing to the
    // signal. In the processBlock method we'll need to give this value to the
    // host.
    std::atomic<int> latency = 0;

    // The gate states help us keep track of which state each of the gates are
    // in. When the gates are in the process of opening, we don't want them to
    // be closed again until they've fully opened.
    enum class GateState
    {
        Closed,
        Opening,
        Open,
        Closing
    };

    std::vector<GateState> gateStates;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GateProcessor)
};
