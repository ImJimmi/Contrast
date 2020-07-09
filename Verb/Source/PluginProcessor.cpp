#include "PluginProcessor.h"
#include "PluginEditor.h"

//======================================================================================================================
PluginProcessor::PluginProcessor()
    :   AudioProcessor(
            BusesProperties().withInput("Input", AudioChannelSet::stereo(), true)
                             .withOutput("Output", AudioChannelSet::stereo(), true)
        ),
        apvts(*this, &undoManager, String(JucePlugin_Name).replace(" ", "_").toUpperCase(), createParameterLayout()),
        size(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("size"))),
        damping(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("damping"))),
        wet(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("wet"))),
        dry(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("dry"))),
        width(*dynamic_cast<AudioParameterFloat*>(apvts.getParameter("width")))
{
    DBG(JucePlugin_Name << " v" << JucePlugin_VersionString);
}

PluginProcessor::~PluginProcessor()
{
}

//======================================================================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int blockSize)
{
    DBG("\tSample rate:        " << sampleRate << "Hz");
    DBG("\tSamples per block:  " << blockSize);

    reverb.setSampleRate(sampleRate);
    reverb.reset();
}

void PluginProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    // Nothing to process. Chances are this will never be needed, but who knows
    // what weird stuff some hosts might do.
    if (numChannels == 0)
        return;

    // Update the reverb parameters.
    Reverb::Parameters parameters;
    parameters.roomSize = size;
    parameters.damping = damping;
    parameters.wetLevel = wet;
    parameters.dryLevel = dry;
    parameters.width = width;
    reverb.setParameters(parameters);

    // Fetch the left channel data. This will also be the mono channel if
    // there's only one channel
    auto leftChannelData = buffer.getWritePointer(0);

    if (numChannels == 1)
    {
        // Process for mono.
        reverb.processMono(leftChannelData, numSamples);
    }
    else
    {
        // If there's at least 2 channels, fetch the right channel data and then
        // processor for stereo. Ignore any other channels.
        auto rightChannelData = buffer.getWritePointer(1);
        reverb.processStereo(leftChannelData, rightChannelData, numSamples);
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
    "HUGE",
    "SUBTLE",
    "TASTFUL"
};

void PluginProcessor::setCurrentPreset(int presetIndex)
{
    // The setValueNotifyingHost() method must only ever be called from the
    // message thread as the host may allocate memory or post a message:
    // https://forum.juce.com/t/calling-setvaluenotifyinghost-from-processblock/26073/9
    JUCE_ASSERT_MESSAGE_THREAD;

    auto newSize = (float)size;
    auto newDamping = (float)damping;
    auto newWet = (float)wet;
    auto newDry = (float)dry;
    auto newWidth = (float)width;

    if (presetIndex == 0)
    {
        newSize = 0.5f;
        newDamping = 0.5f;
        newWet = 0.33f;
        newDry = 0.4f;
        newWidth = 0.75f;
    }
    else if (presetIndex == 1)
    {
        newSize = 0.9f;
        newDamping = 0.6f;
        newWet = 0.4f;
        newDry = 0.4f;
        newWidth = 1.f;
    }
    else if (presetIndex == 2)
    {
        newSize = 0.2f;
        newDamping = 0.35f;
        newWet = 0.2f;
        newDry = 0.75f;
        newWidth = 0.5f;
    }
    else if (presetIndex == 3)
    {
        newSize = 0.4f;
        newDamping = 0.8f;
        newWet = 0.3f;
        newDry = 0.5f;
        newWidth = 0.85f;
    }

    // Need to make sure we call beginChangeGesture() and endChangeGesture() on
    // the parameters before and after we change the parameter's value. This is
    // to ensure the host handle automation properly:
    // https://docs.juce.com/master/classAudioProcessorParameter.html#ac9b67f35339db50d2bd9a026d89390e1
    size.beginChangeGesture();
    size.setValueNotifyingHost(size.getNormalisableRange().convertTo0to1(newSize));
    size.endChangeGesture();

    damping.beginChangeGesture();
    damping.setValueNotifyingHost(damping.getNormalisableRange().convertTo0to1(newDamping));
    damping.endChangeGesture();

    wet.beginChangeGesture();
    wet.setValueNotifyingHost(wet.getNormalisableRange().convertTo0to1(newWet));
    wet.endChangeGesture();

    dry.beginChangeGesture();
    dry.setValueNotifyingHost(dry.getNormalisableRange().convertTo0to1(newDry));
    dry.endChangeGesture();

    width.beginChangeGesture();
    width.setValueNotifyingHost(width.getNormalisableRange().convertTo0to1(newWidth));
    width.endChangeGesture();

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
    auto sizeParam = std::make_unique<AudioParameterFloat>(
        "size",
        "Size",
        NormalisableRange<float>(0.f, 1.f),
        0.5f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return contrast::pretifyValue(value * 100.f, 3) + "%";
        },
        [this](const String& text) -> float {
            return text.getFloatValue() / 100.f;
        }
    );

    auto dampingParam = std::make_unique<AudioParameterFloat>(
        "damping",
        "Damping",
        NormalisableRange<float>(0.f, 1.f),
        0.5f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return contrast::pretifyValue(value * 100.f, 3) + "%";
        },
        [this](const String& text) -> float {
            return text.getFloatValue() / 100.f;
        }
    );

    auto wetParam = std::make_unique<AudioParameterFloat>(
        "wet",
        "Wet",
        NormalisableRange<float>(0.f, 1.f),
        0.33f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return contrast::pretifyValue(value * 100.f, 3) + "%";
        },
        [this](const String& text) -> float {
            return text.getFloatValue() / 100.f;
        }
    );

    auto dryParam = std::make_unique<AudioParameterFloat>(
        "dry",
        "Dry",
        NormalisableRange<float>(0.f, 1.f),
        0.4f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return contrast::pretifyValue(value * 100.f, 3) + "%";
        },
        [this](const String& text) -> float {
            return text.getFloatValue() / 100.f;
        }
    );

    auto widthParam = std::make_unique<AudioParameterFloat>(
        "width",
        "Width",
        NormalisableRange<float>(0.f, 1.f),
        0.75f,
        String(),
        AudioProcessorParameter::genericParameter,
        [this](float value, int) -> String {
            return contrast::pretifyValue(value * 100.f, 3) + "%";
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
        std::move(sizeParam), std::move(dampingParam), std::move(wetParam),
        std::move(dryParam), std::move(widthParam)
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
