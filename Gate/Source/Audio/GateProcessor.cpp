#include "GateProcessor.h"

#include "../GUI/GateEditor.h"

//======================================================================================================================
GateProcessor::GateProcessor()
    :   contrast::PluginProcessor(createParameterLayout(), createDefaultProperties()),
        threshold(*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Gate::ParameterIDs::THRESHOLD))),
        attack(   *dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Gate::ParameterIDs::ATTACK))),
        release(  *dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Gate::ParameterIDs::RELEASE)))
{
    // Need to listen for changes to the attack parameter so we can change the
    // length of the delay lines accordingly.
    getAPVTS().addParameterListener(Gate::ParameterIDs::ATTACK, this);
}

GateProcessor::~GateProcessor()
{
    // Make sure to remove this as a listener to the APVTS.
    getAPVTS().removeParameterListener(Gate::ParameterIDs::ATTACK, this);
}

//======================================================================================================================
void GateProcessor::prepareToPlay(double sampleRate, int /* blockSize */)
{
    // Make sure the vectors have been resized to fit the current number of
    // channels.
    numChannelsChanged();

    // Initialise the current peak followers
    for (auto& follower : currentPeakFollowers)
    {
        follower.reset(new contrast::EnvelopeFollower(static_cast<float>(sampleRate)));
        jassert(follower);

        // We'll use an attack time of 0ms so that the envelope instantly jumps
        // up to match peaks.
        follower->setAttackTime(0.f);

        // A release time of 100ms means the envelope will be smoothed a bit but
        // will still closely follow the envelope of the audio signal.
        follower->setReleaseTime(100.f);
    }

    // Initialise the delayed peak followers
    for (auto& follower : delayedPeakFollowers)
    {
        follower.reset(new contrast::EnvelopeFollower(static_cast<float>(sampleRate)));
        jassert(follower);

        follower->setAttackTime(0.f);
        follower->setReleaseTime(100.f);
    }

    updateDelayLines();
}

void GateProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    process(buffer, false);
}

void GateProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    process(buffer, true);
}

void GateProcessor::releaseResources()
{
    // Clear all the vectors.
    currentPeakFollowers  .clear();
    delayedPeakFollowers  .clear();
    gates                 .clear();
    delayLines            .clear();
    gateStates            .clear();
}

void GateProcessor::numChannelsChanged()
{
    // Since we specified to only allow configurations with the same number of
    // input and output channels, we can use either the input or the output bus
    // to find the total number of available audio channels.
    const auto numChannels = static_cast<std::size_t> (juce::jmax(getTotalNumInputChannels(),
                                                                  getTotalNumOutputChannels()));

    currentPeakFollowers  .resize(numChannels);
    delayedPeakFollowers  .resize(numChannels);
    gates                 .resize(numChannels);
    gateStates            .resize(numChannels);
    delayLines            .resize(numChannels);

    const auto capacity = contrast::ceil<std::size_t>(Gate::releaseMax<double> * getSampleRate() * 0.001);

    for (auto& delayLine : delayLines)
        delayLine.reset(new contrast::DelayLine<float>(capacity));
}

//======================================================================================================================
juce::StringArray GateProcessor::getPresetNames() const
{
    return {
        "<DEFAULT>",
        "KICK",
        "SNARE",
        "GUITAR",
        "VOCALS"
    };
}

void GateProcessor::presetChoiceChanged(int presetIndex)
{
    auto newThreshold = static_cast<float>(threshold);
    auto newAttack    = static_cast<float>(attack);
    auto newRelease   = static_cast<float>(release);

    if (presetIndex == 0)
    {
        newThreshold = -60.f;
        newAttack = 20.f;
        newRelease = 500.f;
    }
    else if (presetIndex == 1)
    {
        newThreshold = -6.f;
        newAttack = 5.f;
        newRelease = 250.f;
    }
    else if (presetIndex == 2)
    {
        newThreshold = -14.f;
        newAttack = 10.f;
        newRelease = 350.f;
    }
    else if (presetIndex == 3)
    {
        newThreshold = -30.f;
        newAttack = 50.f;
        newRelease = 750.f;
    }
    else if (presetIndex == 4)
    {
        newThreshold = -12.f;
        newAttack = 100.f;
        newRelease = 1000.f;
    }

    // Need to make sure we call beginChangeGesture() and endChangeGesture() on
    // the parameters before and after we change the parameter's value. This is
    // to ensure the host handle automation properly:
    // https://docs.juce.com/master/classAudioProcessorParameter.html#ac9b67f35339db50d2bd9a026d89390e1
    threshold.beginChangeGesture();
    threshold.setValueNotifyingHost(threshold.getNormalisableRange().convertTo0to1(newThreshold));
    threshold.endChangeGesture();

    attack.beginChangeGesture();
    attack.setValueNotifyingHost(attack.getNormalisableRange().convertTo0to1(newAttack));
    attack.endChangeGesture();

    release.beginChangeGesture();
    release.setValueNotifyingHost(release.getNormalisableRange().convertTo0to1(newRelease));
    release.endChangeGesture();
}

//======================================================================================================================
void GateProcessor::parameterChanged(const juce::String& parameterID, float /* newValue */)
{
    // Update the delay lines when the attack parameter changes.
    if (parameterID == Gate::ParameterIDs::ATTACK)
        updateDelayLines();
}

//======================================================================================================================
juce::AudioProcessorValueTreeState::ParameterLayout GateProcessor::createParameterLayout() const
{
    auto thresholdParam = std::make_unique<juce::AudioParameterFloat>(
        Gate::ParameterIDs::THRESHOLD,
        "Threshold",
        juce::NormalisableRange<float>(-60.f, 0.f),
        -60.f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) -> juce::String {
            if (value == -60.f)
                return "-INFdB";

            auto text = contrast::pretifyValue(value, 3) + "dB";

            if (text[0] != '-')
                text = "-" + text;

            return text;
        },
        [](const juce::String& text) -> float {
            return text.getFloatValue();
        }
    );

    auto attackParam = std::make_unique<juce::AudioParameterFloat>(
        Gate::ParameterIDs::ATTACK,
        "Attack",
        juce::NormalisableRange<float>(1.f, 300.f),
        20.f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) -> juce::String {
            return contrast::pretifyValue(value, 3) + "ms";
        },
        [](const juce::String& text) -> float {
            return text.getFloatValue();
        }
    );

    auto releaseParam = std::make_unique<juce::AudioParameterFloat>(
        Gate::ParameterIDs::RELEASE,
        "Release",
        juce::NormalisableRange<float>(Gate::releaseMin<float>, Gate::releaseMax<float>),
        500.f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) -> juce::String {
            if (value < 1000.f)
                return contrast::pretifyValue(value, 3) + "ms";

            // Display as seconds if the value is over 1000 ms
            return contrast::pretifyValue(value / 1000.f, 3) + "s";
        },
        [](const juce::String& text) -> float {
            if (text.endsWith("ms"))
                return text.getFloatValue();
            else if (text.endsWith("s"))
                return text.getFloatValue() * 1000.f;

            // For value where the units aren't specified (such as from user
            // input), assume that values less than 20 is in seconds, and
            // anything else is in milliseconds
            auto value = text.getFloatValue();

            if (value < 20.f)
                return value * 1000.f;

            return value;
        }
    );

    // In this plugin we only have one, unnamed group that all of our parameters
    // we live in.
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> groups;
    groups.push_back(std::make_unique<juce::AudioProcessorParameterGroup>(
        "gate", "Gate", "",
        std::move(thresholdParam),
        std::move(attackParam),
        std::move(releaseParam)
    ));

    return { groups.begin(), groups.end() };
}

juce::ValueTree GateProcessor::createDefaultProperties() const
{
    juce::ValueTree tree("ADDITIONAL_PROPERTIES");
    return tree;
}

//======================================================================================================================
void GateProcessor::process(juce::AudioBuffer<float>& buffer, bool isBypassed)
{
    const juce::ScopedNoDenormals noDenormals;

    const auto numChannels = static_cast<std::size_t>(buffer.getNumChannels());
    const auto numSamples = static_cast<std::size_t>(buffer.getNumSamples());

    jassert(currentPeakFollowers.size() >= numChannels);
    jassert(delayedPeakFollowers.size() >= numChannels);

    const auto sampleRate = getSampleRate();

    // Make sure to tell the host how much delay our plugin is introducing so
    // it can act accordingly.
    setLatencySamples(static_cast<int>(latency));

    for (std::size_t channel = 0; channel < numChannels; channel++)
    {
        auto channelData = buffer.getWritePointer(static_cast<int>(channel));

        jassert(currentPeakFollowers[channel] != nullptr);
        jassert(delayedPeakFollowers[channel] != nullptr);
        jassert(delayLines[channel]           != nullptr);

        for (std::size_t i = 0; i < numSamples; i++)
        {
            auto nonDelayedInput = channelData[i];

            // Get the envelope, in decibels, of the current peak. This is the
            // non delayed signal and so is ahead of time since we've told the
            // host we're introducing some latency.
            auto currentEnvelope = currentPeakFollowers[channel]->processSample(nonDelayedInput);
            currentEnvelope = juce::Decibels::gainToDecibels(currentEnvelope);

            // Get the delayed input - this is the input N samples ago when we
            // have N samples of latency (AKA the actual 'live' sample).
            auto delayedInput = delayLines[channel]->read();
            delayLines[channel]->write(nonDelayedInput);

            // Get the envelope, in decibels, of the delayed input. This is the
            // 'live' signal essentially.
            auto delayedEnvelope = delayedPeakFollowers[channel]->processSample(delayedInput);
            delayedEnvelope = juce::Decibels::gainToDecibels(delayedEnvelope, -60.f);

            auto previousGain = gates[channel].getCurrentValue();

            // Only modify the gate's state if it's NOT currently in the process
            // of opening. Otherwise we might start closing the gate before it
            // fully opening and the peaks won't have full gain.
            if (gateStates[channel] != GateState::Opening)
            {
                // Enter attacking phase (gate opening) if the CURRENT envelope
                // (ahead of time) is above the threshold.
                if (currentEnvelope > threshold)
                {
                    gateStates[channel] = GateState::Opening;
                    gates[channel].reset(sampleRate, attack * 0.001);
                    gates[channel].setCurrentAndTargetValue(previousGain);
                    gates[channel].setTargetValue(1.f);
                }
                // Don't start closing the gate again until the DELAYED envelope
                // has fallen below the threshold. This allows the gate to fully
                // open before starting to close again.
                // Also don't tell the gate to start closing if it's already
                // in the process of closing because the release time will be
                // longer than intended.
                else if (delayedEnvelope < threshold && gateStates[channel] != GateState::Closing)
                {
                    gateStates[channel] = GateState::Closing;
                    gates[channel].reset(sampleRate, release * 0.001);
                    gates[channel].setCurrentAndTargetValue(previousGain);
                    gates[channel].setTargetValue(0.f);
                }
            }

            // Get the square root of the gate's next value - this means the
            // slope is more linear when converted to decibels.
            const auto gain = std::sqrt(gates[channel].getNextValue());

            // Update the gate's state if it's very nearly fully closed or very
            // nearly fully open. This is to accomodate for any precision loss
            // where the gain might not be exactly 0 or 1 when the interpolation
            // finishes.
            if (gain >= 0.999f && gateStates[channel] == GateState::Opening)
                gateStates[channel] = GateState::Open;
            else if (gain < 0.001f && gateStates[channel] == GateState::Closing)
                gateStates[channel] = GateState::Closed;

            // Apply the gain we've calculated to the delayed input.
            if (!isBypassed)
                channelData[i] = gain * delayedInput;
        }
    }
}

void GateProcessor::updateDelayLines()
{
    // Recalculate the latency value which is the number of samples in the
    // attack time
    latency = contrast::round<std::size_t>(attack * getSampleRate() * 0.001f);

    // Resize the delay lines to match the latency
    for (auto& delayLine : delayLines)
    {
        if (delayLine != nullptr)
            delayLine->setLength(latency);
    }
}

//======================================================================================================================
juce::AudioProcessorEditor* GateProcessor::createEditor()
{
    return new GateEditor(*this);
}

//======================================================================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GateProcessor();
}
