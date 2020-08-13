#include "PluginProcessor.h"
#include "PluginEditor.h"

//======================================================================================================================
PluginProcessor::PluginProcessor()
    :   AudioProcessor(
            BusesProperties().withInput("Input", AudioChannelSet::stereo(), true)
                             .withOutput("Output", AudioChannelSet::stereo(), true)
        ),
        apvts(*this, &undoManager, String(JucePlugin_Name).replace(" ", "_").toUpperCase(), createParameterLayout()),
        semitones(*dynamic_cast<AudioParameterInt*>(  apvts.getParameter("semitones"))),
        cents(    *dynamic_cast<AudioParameterFloat*>(apvts.getParameter("cents"))),
        mix(      *dynamic_cast<AudioParameterFloat*>(apvts.getParameter("mix")))
{
    DBG(JucePlugin_Name << " v" << JucePlugin_VersionString);

    apvts.addParameterListener("semitones", this);
    apvts.addParameterListener("cents",     this);
    apvts.addParameterListener("mix",       this);
}

PluginProcessor::~PluginProcessor()
{
    apvts.removeParameterListener("semitones",  this);
    apvts.removeParameterListener("cents",      this);
    apvts.removeParameterListener("mix",        this);
}

//======================================================================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int blockSize)
{
    DBG("\tSample rate:        " << sampleRate << "Hz");
    DBG("\tSamples per block:  " << blockSize);

    // Make sure the vectors have been resized to fit the current number of
    // channels.
    numChannelsChanged();

    // Initialise the compressors
    for (auto& pitShift : pitShifters)
    {
        pitShift.reset(new contrast::PitchShifter(5024, sampleRate, static_cast<uint32>(blockSize)));
        pitShift->setShift(2.f);
        jassert(pitShift);
    }

    // Make sure the compressors are initialised with the current parameters
    // by faking some parameter changed calls.
    parameterChanged("semitones",   semitones);
    parameterChanged("cents",       cents);
}

void PluginProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    jassert(pitShifters.size() == numChannels);

    for (int channel = 0; channel < numChannels; channel++)
    {
        jassert(pitShifters[channel]);

        auto channelData = buffer.getWritePointer(channel);

        for (int i = 0; i < numSamples; i++)
            channelData[i] = pitShifters[channel]->processSample(channelData[i]);
    }

    setLatencySamples(pitShifters[0]->getDelayLength() / 2);
}

void PluginProcessor::releaseResources()
{
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

    pitShifters.resize(numChannels);
}

//======================================================================================================================
void PluginProcessor::parameterChanged(const String& parameterID, float)
{
    if (parameterID == "semitones" || parameterID == "cents")
    {
        for (auto& pitShift : pitShifters)
        {
            jassert(pitShift);
            pitShift->setShift(semitones, cents);
        }
    }
    else if (parameterID == "mix")
    {
        for (auto& pitShift : pitShifters)
        {
            jassert(pitShift);
            pitShift->setMix(mix);
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
    "<DEFAULT>"
};

void PluginProcessor::setCurrentPreset(int presetIndex)
{
    // The setValueNotifyingHost() method must only ever be called from the
    // message thread as the host may allocate memory or post a message:
    // https://forum.juce.com/t/calling-setvaluenotifyinghost-from-processblock/26073/9
    JUCE_ASSERT_MESSAGE_THREAD;

    auto newSemitones = static_cast<int>(semitones);
    auto newCents =     static_cast<float>(cents);

    // Need to make sure we call beginChangeGesture() and endChangeGesture() on
    // the parameters before and after we change the parameter's value. This is
    // to ensure the host handle automation properly:
    // https://docs.juce.com/master/classAudioProcessorParameter.html#ac9b67f35339db50d2bd9a026d89390e1
    semitones.beginChangeGesture();
    semitones.setValueNotifyingHost(semitones.getNormalisableRange().convertTo0to1(newSemitones));
    semitones.endChangeGesture();

    cents.beginChangeGesture();
    cents.setValueNotifyingHost(cents.getNormalisableRange().convertTo0to1(newCents));
    cents.endChangeGesture();

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
    auto semitonesParam = std::make_unique<AudioParameterInt>(
        "semitones",
        "Semitones",
        -24, 24, 0,
        String(),
        [this](int value, int) -> String {
            return String(value);
        },
        [this](const String& text) -> int {
            return text.getIntValue();
        }
    );
    
    auto centsParam = std::make_unique<AudioParameterFloat>(
        "cents",
        "Cents",
        NormalisableRange<float>(-100.f, 100.f),
        0.f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return contrast::pretifyValue(value, 3) + "%";
        },
        [this](const String& text) -> float {
            return text.getFloatValue();
        }
    );

    auto mixParam = std::make_unique<AudioParameterFloat>(
        "mix",
        "Mix",
        NormalisableRange<float>(0.f, 1.f),
        1.f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return contrast::pretifyValue(value * 100.f, 3);
        },
        [this](const String& text) -> float {
            return text.getFloatValue() / 100.f;
        }
    );

    // In this plugin we only have one, unnamed group that all of our parameters
    // we live in.
    std::vector<std::unique_ptr<AudioProcessorParameterGroup>> groups;
    groups.push_back(std::make_unique<AudioProcessorParameterGroup>(
        "", "", "",
        std::move(semitonesParam), std::move(centsParam), std::move(mixParam)
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
