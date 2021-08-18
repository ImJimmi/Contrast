#include "VerbProcessor.h"

#include "../GUI/VerbEditor.h"
#include "../Verb.h"

//======================================================================================================================
VerbProcessor::VerbProcessor()
    :   contrast::PluginProcessor(createParameterLayout(), createDefaultProperties()),
        size   (*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Verb::ParameterIDs::SIZE))),
        damping(*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Verb::ParameterIDs::DAMPING))),
        wet    (*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Verb::ParameterIDs::WET))),
        dry    (*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Verb::ParameterIDs::DRY))),
        width  (*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Verb::ParameterIDs::WIDTH)))
{
}

VerbProcessor::~VerbProcessor()
{
}

//======================================================================================================================
void VerbProcessor::prepareToPlay(double sampleRate, int blockSize)
{
    reverb.setSampleRate(sampleRate);
    reverb.reset();
}

void VerbProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    // Nothing to process. Chances are this will never be needed, but since
    // we're accessing channels in the buffer using hard-coded indices it will
    // mean we'll never get any runtime errors (hopefully).
    if (numChannels == 0)
        return;

    // Update the reverb parameters.
    juce::Reverb::Parameters parameters;
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

void VerbProcessor::releaseResources()
{
    reverb.reset();
}

#if JUCE_DEBUG
bool VerbProcessor::isBusesLayoutSupported(const BusesLayout&) const
{
    return true;
}
#else
bool VerbProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    // Only support mono or stereo layouts where the input and output layouts
    // match.
    return layout.inputBuses.size() <= 2
           && layout.inputBuses.size() == layout.outputBuses.size();
}
#endif

//======================================================================================================================
juce::AudioProcessorEditor* VerbProcessor::createEditor()
{
    return new VerbEditor(*this);
}

//======================================================================================================================
juce::StringArray VerbProcessor::getPresetNames() const
{
    return {
        "<DEFAULT>",
        "HUGE",
        "SUBTLE",
        "TASTEFUL"
    };
}

//======================================================================================================================
juce::AudioProcessorValueTreeState::ParameterLayout VerbProcessor::createParameterLayout() const
{
    auto sizeParam = std::make_unique<juce::AudioParameterFloat>(
        Verb::ParameterIDs::SIZE,
        "Size",
        juce::NormalisableRange<float>(0.f, 1.f),
        0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [this](float value, int) -> juce::String {
            return contrast::pretifyValue(value * 100.f, 3) + "%";
        },
        [this](const juce::String& text) -> float {
            return text.getFloatValue() / 100.f;
        }
    );

    auto dampingParam = std::make_unique<juce::AudioParameterFloat>(
        Verb::ParameterIDs::DAMPING,
        "Damping",
        juce::NormalisableRange<float>(0.f, 1.f),
        0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [this](float value, int) -> juce::String {
            return contrast::pretifyValue(value * 100.f, 3) + "%";
        },
        [this](const juce::String& text) -> float {
            return text.getFloatValue() / 100.f;
        }
    );

    auto wetParam = std::make_unique<juce::AudioParameterFloat>(
        Verb::ParameterIDs::WET,
        "Wet",
        juce::NormalisableRange<float>(0.f, 1.f),
        0.33f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [this](float value, int) -> juce::String {
            return contrast::pretifyValue(value * 100.f, 3) + "%";
        },
        [this](const juce::String& text) -> float {
            return text.getFloatValue() / 100.f;
        }
    );

    auto dryParam = std::make_unique<juce::AudioParameterFloat>(
        Verb::ParameterIDs::DRY,
        "Dry",
        juce::NormalisableRange<float>(0.f, 1.f),
        0.4f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [this](float value, int) -> juce::String {
            return contrast::pretifyValue(value * 100.f, 3) + "%";
        },
        [this](const juce::String& text) -> float {
            return text.getFloatValue() / 100.f;
        }
    );

    auto widthParam = std::make_unique<juce::AudioParameterFloat>(
        Verb::ParameterIDs::WIDTH,
        "Width",
        juce::NormalisableRange<float>(0.f, 1.f),
        0.75f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [this](float value, int) -> juce::String {
            return contrast::pretifyValue(value * 100.f, 3) + "%";
        },
        [this](const juce::String& text) -> float {
            return text.getFloatValue() / 100.f;
        }
    );

    // In this plugin we only have one, unnamed group that all of our parameters
    // will live in.
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> groups;
    groups.push_back(std::make_unique<juce::AudioProcessorParameterGroup>(
        "verb", "Verb", "",
        std::move(sizeParam), std::move(dampingParam), std::move(wetParam),
        std::move(dryParam), std::move(widthParam)
    ));

    return { groups.begin(), groups.end() };
}

juce::ValueTree VerbProcessor::createDefaultProperties() const
{
    juce::ValueTree tree("ADDITIONAL_PROPERTIES");
    return tree;
}

void VerbProcessor::presetChoiceChanged(int presetIndex)
{
    auto newSize    = static_cast<float>(size);
    auto newDamping = static_cast<float>(damping);
    auto newWet     = static_cast<float>(wet);
    auto newDry     = static_cast<float>(dry);
    auto newWidth   = static_cast<float>(width);

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
}

//======================================================================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VerbProcessor();
}
