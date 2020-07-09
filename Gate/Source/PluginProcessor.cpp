#include "PluginProcessor.h"
#include "PluginEditor.h"

//======================================================================================================================
PluginProcessor::PluginProcessor()
    :   AudioProcessor(
            BusesProperties().withInput("Input", AudioChannelSet::stereo(), true)
                             .withOutput("Output", AudioChannelSet::stereo(), true)
        ),
        apvts(*this, &undoManager, String(JucePlugin_Name).replace(" ", "_").toUpperCase(), createParameterLayout()),
        threshold(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("threshold"))),
        attack(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("attack"))),
        release(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("release")))
{
    DBG(JucePlugin_Name << " v" << JucePlugin_VersionString);

    // Need to listen for changes to the attack parameter so we can change the
    // length of the delay lines accordingly.
    apvts.addParameterListener("attack",    this);
}

PluginProcessor::~PluginProcessor()
{
    apvts.removeParameterListener("attack", this);
}

//======================================================================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int blockSize)
{
    DBG("\tSample rate:        " << sampleRate << "Hz");
    DBG("\tSamples per block:  " << blockSize);

    // Make sure the vectors have been resized to fit the current number of
    // channels.
    numChannelsChanged();

    // Initialise the current peak followers
    for (auto& follower : currentPeakFollowers)
    {
        follower.reset(new contrast::EnvelopeFollower(static_cast<float>(sampleRate)));
        jassert(follower);

        follower->setAttackTime(0.f);
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

    // Make sure the delay lines have been initialised by faking a parameter
    // change call.
    parameterChanged("attack",  attack);
}

void PluginProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    jassert(currentPeakFollowers.size() >= numChannels);
    jassert(delayedPeakFollowers.size() >= numChannels);

    const auto sampleRate = static_cast<float>(getSampleRate());

    // Make sure to tell the host how much delay our plugin is introducing so
    // it can act accordingly.
    setLatencySamples(latency);

    for (int channel = 0; channel < numChannels; channel++)
    {
        auto channelData = buffer.getWritePointer(channel);

        jassert(currentPeakFollowers[channel]);
        jassert(delayedPeakFollowers[channel]);

        for (int i = 0; i < numSamples; i++)
        {
            auto nonDelayedInput = channelData[i];

            // Get the envelope, in decibels, of the current peak. This is the
            // non delayed signal and so is ahead of time since we've told the
            // host we're introducing some latency.
            auto currentEnvelope = currentPeakFollowers[channel]->processSample(nonDelayedInput);
            currentEnvelope = Decibels::gainToDecibels(currentEnvelope, -60.f);

            // Get the delayed input - this is the input N samples ago when we
            // have N samples of latency (AKA the actual 'live' sample).
            auto delayedInput = delayLines[channel][delayLinesWriteIndices[channel]];
            delayLines[channel][delayLinesWriteIndices[channel]] = nonDelayedInput;

            // Must increment the index and wrap it back to 0 if it has exceeded
            // the length of the delay line. This allows the linear vectors to
            // work as ring buffers.
            if (++delayLinesWriteIndices[channel] >= delayLines[channel].size())
                delayLinesWriteIndices[channel] = 0;

            // Get the envelope, in decibels, of the delayed envelope. This is
            // the 'live' signal essentially.
            auto delayedEnvelope = delayedPeakFollowers[channel]->processSample(delayedInput);
            delayedEnvelope = Decibels::gainToDecibels(delayedEnvelope, -60.f);

            auto previousGain = gates[channel].getCurrentValue();

            // Only modify the gate's state if it's NOT currently in the process
            // of opening. Otherwise we might start closing the gate beofre it
            // fully opening and the peaks won't have full gain.
            if (gateStates[channel] != GateState::Opening)
            {
                // Enter attacking phase (gate opening) if the CUIRRENT envelope
                // (ahead of time) is above the threshold.
                if (currentEnvelope > threshold)
                {
                    gateStates[channel] = GateState::Opening;
                    gates[channel].reset(static_cast<double>(sampleRate), attack * 0.001);
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
                    gates[channel].reset(static_cast<double>(sampleRate), release * 0.001);
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
            channelData[i] = gain * delayedInput;
        }
    }
}

void PluginProcessor::releaseResources()
{
    // Clear all the vectors.
    currentPeakFollowers.clear();
    delayedPeakFollowers.clear();
    gates.clear();
    delayLines.clear();
    delayLinesWriteIndices.clear();
    gateStates.clear();
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JUCE_DEBUG
    // When running in debug we'll allow any sort of channel layout to make
    // testing a little easier.
    return true;
#else
    // When not in debug, we'll allow any channel setup where the input has the
    // same number of channels as the output.
    return layouts.inputBuses.size() == layouts.outputBuses.size();
#endif
}

void PluginProcessor::numChannelsChanged()
{
    // Since we specified to only allow configurations with the same number of
    // input and output channels, we can use either the input or the output bus
    // to find the total number of available audio channels.
    const auto numChannels = getTotalNumInputChannels();
    DBG("\tNumber of channels: " << numChannels);

    currentPeakFollowers.resize(numChannels);
    delayedPeakFollowers.resize(numChannels);
    gates.resize(numChannels);
    gateStates.resize(numChannels);
    delayLines.resize(numChannels);
    delayLinesWriteIndices.resize(numChannels, 0);
}

//======================================================================================================================
void PluginProcessor::parameterChanged(const String& parameterID, float newValue)
{
    if (parameterID == "attack")
    {
        // Recalculate the latency value which is the number of samples in the
        // attack time
        latency = (int)std::round(attack * getSampleRate() * 0.001f);

        // Resize the delay lines to match the latency
        for (auto& delayLine : delayLines)
            delayLine.resize(latency, 0.f);

        // Wrap the indicies if they're behond the length of the delay line.
        for (auto& index : delayLinesWriteIndices)
        {
            if (index >= latency)
                index = 0;
        }
    }
}

//======================================================================================================================
void PluginProcessor::getStateInformation(MemoryBlock& destData)
{
    // Create a copy of the APVTS's state.
    auto state = apvts.copyState();

    {
        const ScopedLock lock(propertiesMutex);

        // Add the additional properties to the state.
        for (auto& property : additionalProperties)
            state.setProperty(property.name, property.value, nullptr);
    }

    // Convert the state to XML and send it to the provided memory block.
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Parse the provided data as XML
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName(apvts.state.getType()))
        {
            // Create a ValueTree object from the XML element and replace the
            // APVTS's state with it.
            auto state = ValueTree::fromXml(*xml);
            apvts.replaceState(state);

            const ScopedLock lock(propertiesMutex);

            // Retrieve the additional properties from the new state.
            for (int i = 0; i < state.getNumProperties(); i++)
            {
                const auto name = state.getPropertyName(i);
                additionalProperties.set(name, state[name]);
            }
        }
    }
}

//======================================================================================================================
const StringArray PluginProcessor::presetNames = {
    "<DEFAULT>",
    "KICK",
    "SNARE",
    "GUITAR",
    "VOCALS"
};

void PluginProcessor::setCurrentPreset(int presetIndex)
{
    // The setValueNotifyingHost() method must only ever be called from the
    // message thread as the host may allocate memory or post a message:
    // https://forum.juce.com/t/calling-setvaluenotifyinghost-from-processblock/26073/9
    JUCE_ASSERT_MESSAGE_THREAD;

    auto newThreshold = static_cast<float>(threshold);
    auto newAttack = static_cast<float>(attack);
    auto newRelease = static_cast<float>(release);

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

    setAdditionalProperty("presetIndex", presetIndex);
}

//======================================================================================================================
void PluginProcessor::setAdditionalProperty(const Identifier& name, const var& newValue)
{
    const ScopedLock lock(propertiesMutex);
    additionalProperties.set(name, newValue);
}

var PluginProcessor::getAdditionalProperty(const Identifier& name, const var& defaultValue) const
{
    const ScopedLock lock(propertiesMutex);
    return additionalProperties.getWithDefault(name, defaultValue);
}

//======================================================================================================================
AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    auto thresholdParam = std::make_unique<AudioParameterFloat>(
        "threshold",
        "Threshold",
        NormalisableRange<float>(-60.f, 0.f),
        -60.f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            if (value == -60.f)
                return "-INFdB";

            auto text = contrast::pretifyValue(value, 3) + "dB";

            if (text[0] != '-')
                text = "-" + text;

            return text;
        },
        [this](const String& text) -> float {
            return text.getFloatValue();
        }
    );

    auto attackParam = std::make_unique<AudioParameterFloat>(
        "attack",
        "Attack",
        NormalisableRange<float>(1.f, 300.f),
        20.f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return contrast::pretifyValue(value, 3) + "ms";
        },
        [this](const String& text) -> float {
            return text.getFloatValue();
        }
    );

    auto releaseParam = std::make_unique<AudioParameterFloat>(
        "release",
        "Release",
        NormalisableRange<float>(20.f, 2000.f),
        500.f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            if (value < 1000.f)
                return contrast::pretifyValue(value, 3) + "ms";

            // Display as seconds if the value is over 1000 ms
            return contrast::pretifyValue(value / 1000.f, 3) + "s";
        },
        [this](const String& text) -> float {
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
    std::vector<std::unique_ptr<AudioProcessorParameterGroup>> groups;
    groups.push_back(std::make_unique<AudioProcessorParameterGroup>(
        "", "", "",
        std::move(thresholdParam),
        std::move(attackParam),
        std::move(releaseParam)
    ));

    return { groups.begin(), groups.end() };
}

//======================================================================================================================
AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
}

//======================================================================================================================
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
