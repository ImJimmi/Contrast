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
        ratio(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("ratio"))),
        knee(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("knee"))),
        attack(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("attack"))),
        release(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("release"))),
        gain(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("gain")))
{
    DBG(JucePlugin_Name << " v" << JucePlugin_VersionString);

    apvts.addParameterListener("threshold", this);
    apvts.addParameterListener("ratio",     this);
    apvts.addParameterListener("knee",      this);
    apvts.addParameterListener("attack",    this);
    apvts.addParameterListener("release",   this);
    apvts.addParameterListener("gain",      this);
}

PluginProcessor::~PluginProcessor()
{
    apvts.removeParameterListener("threshold",  this);
    apvts.removeParameterListener("ratio",      this);
    apvts.removeParameterListener("knee",       this);
    apvts.removeParameterListener("attack",     this);
    apvts.removeParameterListener("release",    this);
    apvts.removeParameterListener("gain",       this);
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
    for (auto& compressor : compressors)
    {
        compressor.reset(new contrast::Compressor((float)sampleRate));
        jassert(compressor);
    }

    // Make sure the compressors are initialised with the current parameters
    // by faking some parameter changed calls.
    parameterChanged("threshold",   threshold);
    parameterChanged("ratio",       ratio);
    parameterChanged("knee",        knee);
    parameterChanged("attack",      attack);
    parameterChanged("release",     release);
    parameterChanged("gain",        gain);
}

void PluginProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    jassert(compressors.size() == numChannels);

    for (int channel = 0; channel < numChannels; channel++)
    {
        jassert(compressors[channel]);

        auto channelData = buffer.getWritePointer(channel);

        for (int i = 0; i < numSamples; i++)
            channelData[i] = compressors[channel]->processSample(channelData[i]);
    }
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

    compressors.resize(numChannels);
}

//======================================================================================================================
void PluginProcessor::parameterChanged(const String& parameterID, float newValue)
{
    if (parameterID == "threshold")
    {
        for (auto& compressor : compressors)
        {
            jassert(compressor);
            compressor->setThreshold(newValue);
        }
    }
    else if (parameterID == "ratio")
    {
        for (auto& compressor : compressors)
        {
            jassert(compressor);
            compressor->setRatio(newValue);
        }
    }
    else if (parameterID == "knee")
    {
        for (auto& compressor : compressors)
        {
            jassert(compressor);
            compressor->setKnee(newValue);
        }
    }
    else if (parameterID == "attack")
    {
        for (auto& compressor : compressors)
        {
            jassert(compressor);
            compressor->setAttack(newValue);
        }
    }
    else if (parameterID == "release")
    {
        for (auto& compressor : compressors)
        {
            jassert(compressor);
            compressor->setRelease(newValue);
        }
    }
    else if (parameterID == "gain")
    {
        for (auto& compressor : compressors)
        {
            jassert(compressor);
            compressor->setMakeupGain(newValue);
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
    "PEAK CONTROL",
    "HEAVY",
    "SLOW"
};

void PluginProcessor::setCurrentPreset(int presetIndex)
{
    // The setValueNotifyingHost() method must only ever be called from the
    // message thread as the host may allocate memory or post a message:
    // https://forum.juce.com/t/calling-setvaluenotifyinghost-from-processblock/26073/9
    JUCE_ASSERT_MESSAGE_THREAD;

    auto newThreshold = static_cast<float>(threshold);
    auto newRatio =     static_cast<float>(ratio);
    auto newKnee =      static_cast<float>(knee);
    auto newAttack =    static_cast<float>(attack);
    auto newRelease =   static_cast<float>(release);
    auto newGain =      static_cast<float>(gain);

    if (presetIndex == 0)
    {
        newThreshold = -0.f;
        newRatio = 1.f;
        newKnee = 0.f;
        newAttack = 20.f;
        newRelease = 1000.f;
        newGain = 0.f;
    }
    else if (presetIndex == 1)
    {
        newThreshold = -15.f;
        newRatio = 5.f;
        newKnee = 12.f;
        newAttack = 50.f;
        newRelease = 500.f;
        newGain = 0.f;
    }
    else if (presetIndex == 2)
    {
        newThreshold = -24.f;
        newRatio = 15.f;
        newKnee = 6.f;
        newAttack = 75.f;
        newRelease = 1500.f;
        newGain = 7.f;
    }
    else if (presetIndex == 3)
    {
        newThreshold = -18.f;
        newRatio = 10.f;
        newKnee = 12.f;
        newAttack = 200.f;
        newRelease = 2000.f;
        newGain = 0.f;
    }
    // Need to make sure we call beginChangeGesture() and endChangeGesture() on
    // the parameters before and after we change the parameter's value. This is
    // to ensure the host handle automation properly:
    // https://docs.juce.com/master/classAudioProcessorParameter.html#ac9b67f35339db50d2bd9a026d89390e1
    threshold.beginChangeGesture();
    threshold.setValueNotifyingHost(threshold.getNormalisableRange().convertTo0to1(newThreshold));
    threshold.endChangeGesture();

    ratio.beginChangeGesture();
    ratio.setValueNotifyingHost(ratio.getNormalisableRange().convertTo0to1(newRatio));
    ratio.endChangeGesture();

    knee.beginChangeGesture();
    knee.setValueNotifyingHost(knee.getNormalisableRange().convertTo0to1(newKnee));
    knee.endChangeGesture();

    attack.beginChangeGesture();
    attack.setValueNotifyingHost(attack.getNormalisableRange().convertTo0to1(newAttack));
    attack.endChangeGesture();

    release.beginChangeGesture();
    release.setValueNotifyingHost(release.getNormalisableRange().convertTo0to1(newRelease));
    release.endChangeGesture();

    gain.beginChangeGesture();
    gain.setValueNotifyingHost(gain.getNormalisableRange().convertTo0to1(newGain));
    gain.endChangeGesture();

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
        0.f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            auto text = contrast::pretifyValue(value, 3) + "dB";

            if (text[0] != '-')
                text = "-" + text;

            return text;
        },
        [this](const String& text) -> float {
            return text.getFloatValue();
        }
    );
    
    auto ratioParam = std::make_unique<AudioParameterFloat>(
        "ratio",
        "Ratio",
        NormalisableRange<float>(1.f, 20.f),
        1.f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return "1 : " + contrast::pretifyValue(value, 2);
        },
        [this](const String& text) -> float {
            return text.fromLastOccurrenceOf("1 : ", false, true).getFloatValue();
        }
    );
    
    auto kneeParam = std::make_unique<AudioParameterFloat>(
        "knee",
        "Knee",
        NormalisableRange<float>(0.f, 20.f),
        0.f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return contrast::pretifyValue(value, 3) + "dB";
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
        NormalisableRange<float>(20.f, 5000.f),
        1000.f,
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
    
    auto gainParam = std::make_unique<AudioParameterFloat>(
        "gain",
        "Gain",
        NormalisableRange<float>(-18.f, 18.f),
        0.f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return contrast::pretifyValue(value, 3) + "dB";
        },
        [this](const String& text) -> float {
            return text.getFloatValue();
        }
    );

    // In this plugin we only have one, unnamed group that all of our parameters
    // we live in.
    std::vector<std::unique_ptr<AudioProcessorParameterGroup>> groups;
    groups.push_back(std::make_unique<AudioProcessorParameterGroup>(
        "", "", "",
        std::move(thresholdParam), std::move(ratioParam), std::move(kneeParam),
        std::move(attackParam), std::move(releaseParam), std::move(gainParam)
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
