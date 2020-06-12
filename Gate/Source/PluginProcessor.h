#pragma once

#include "JuceHeader.h"

//======================================================================================================================
class PluginProcessor   :   public AudioProcessor,
                            public AudioProcessorValueTreeState::Listener
{
public:
    //==================================================================================================================
    PluginProcessor();
    ~PluginProcessor();

    //==================================================================================================================
    void prepareToPlay(double, int) override;
    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout&) const override;

    void numChannelsChanged() override;

    //==================================================================================================================
    void parameterChanged(const String&, float) override;

    //==================================================================================================================
    void getStateInformation(MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    //==================================================================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==================================================================================================================
    const String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==================================================================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const String getProgramName(int) override { return {}; }
    void changeProgramName(int, const String&) override {}

    //==================================================================================================================
    /** Replaces the state of the APVTS with that of the specified preset.
        If the index is out of range, this will do nothing.
    */
    void setCurrentPreset(int presetIndex);

    // The names of the available presets in this plugin.
    static const StringArray presetNames;

    //==================================================================================================================
    /** Changes the value of one of the additional properties.
        If no property with the given name is found, this will create a new one.
    */
    void setAdditionalProperty(const Identifier& name, const var& newValue);

    /** Returns the value of the specified additional property.
        If no property with the given name is found, this will return the
        defaultValue, which is an empty var unless specified otherwise.
    */
    var getAdditionalProperty(const Identifier& name, const var& defaultValue = var()) const;

    //==================================================================================================================
    // The AudioProcessorValueTreeState (APVTS) stores our audio parameters and
    // allows us to easily save and reload the state of our plugin.
    AudioProcessorValueTreeState apvts;

private:
    //==================================================================================================================
    /** Creates the audio parameters for our plugin which are passed to the
        APVTS when it is created.
    */
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==================================================================================================================
    UndoManager undoManager;

    // Additonal properties stored with the APVTS that aren't audio parameters
    // Need a mutex to ensure thread safety since they're saved/loaded on the
    // audio thread but most likely used on the message thread.
    CriticalSection propertiesMutex;
    NamedValueSet additionalProperties;

    // Hold references to the 3 parameters for easy access.
    AudioParameterFloat& threshold;
    AudioParameterFloat& attack;
    AudioParameterFloat& release;

    // Two envelopes to follow the current and delayed peaks. This is so we can
    // know when to first open the gate (using the current envelope) and then
    // when to close the gate (using the delayed envelope).
    // We need one follower for each channel, so they're declared as vectors.
    std::vector<std::unique_ptr<contrast::EnvelopeFollower>> currentPeakFollowers;
    std::vector<std::unique_ptr<contrast::EnvelopeFollower>> delayedPeakFollowers;

    // The actual gates which use a linearly smoothed value to allow the gate
    // to gradually open and close, instead of instantly stepping from 0 gain to
    // +1 gain.
    std::vector<SmoothedValue<float, ValueSmoothingTypes::Linear>> gates;

    // The delay lines that allow us to use a look-ahead technique. The indices
    // keep track of which position in the vectors we should write to next.
    std::vector<std::vector<float>> delayLines;
    std::vector<int> delayLinesWriteIndices;

    // The amount of latency, in samples, that our plugin is introducing to the
    // signal. In the processBlock method we'll need to give this value to the
    // host.
    int latency = 0;

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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
