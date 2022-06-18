#include "PressProcessor.h"
#include "../GUI/PressEditor.h"

#include "../Press.h"

//======================================================================================================================
PressProcessor::PressProcessor()
    :   contrast::PluginProcessor(createParameterLayout(), createDefaultProperties()),
        threshold(*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Press::ParameterIDs::THRESHOLD))),
        ratio    (*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Press::ParameterIDs::RATIO))),
        knee     (*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Press::ParameterIDs::KNEE))),
        attack   (*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Press::ParameterIDs::ATTACK))),
        release  (*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Press::ParameterIDs::RELEASE))),
        gain     (*dynamic_cast<juce::AudioParameterFloat*>(getAPVTS().getParameter(Press::ParameterIDs::GAIN)))
{
}

PressProcessor::~PressProcessor()
{
}

//======================================================================================================================
void PressProcessor::prepareToPlay(double sampleRate, int /* blockSize */)
{
    // Make sure the vectors have been resized to fit the current number of
    // channels.
    numChannelsChanged();

    // Initialise the compressors
    for (auto& compressor : compressors)
        compressor.reset(new contrast::Compressor(static_cast<float>(sampleRate)));

    updateCompressors();
}

void PressProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Make sure the compressors are up-to-date.
    updateCompressors();

    const auto numChannels = static_cast<std::size_t>(buffer.getNumChannels());
    const auto numSamples = static_cast<std::size_t>(buffer.getNumSamples());

    jassert(compressors.size() >= numChannels);

    for (std::size_t channel = 0; channel < numChannels; channel++)
    {
        jassert(compressors[channel] != nullptr);

        auto channelData = buffer.getWritePointer(static_cast<int>(channel));

        for (std::size_t i = 0; i < numSamples; i++)
            channelData[i] = compressors[channel]->processSample(channelData[i]);
    }
}

void PressProcessor::releaseResources()
{
    compressors.clear();
}

void PressProcessor::numChannelsChanged()
{
    // Since we specified to only allow configurations with the same number of
    // input and output channels, we can use either the input or the output bus
    // to find the total number of available audio channels.
    const auto numChannels = static_cast<std::size_t>(juce::jmax(getTotalNumInputChannels(),
                                                                 getTotalNumOutputChannels()));
    compressors.resize(numChannels);
}

//======================================================================================================================
juce::StringArray PressProcessor::getPresetNames() const
{
    return {
        "<DEFAULT>",
        "PEAK CONTROL",
        "HEAVY",
        "SLOW"
    };
}

//======================================================================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PressProcessor::createParameterLayout() const
{
    auto thresholdParam = std::make_unique<juce::AudioParameterFloat>(
        Press::ParameterIDs::THRESHOLD,
        "Threshold",
        juce::NormalisableRange<float>(-60.f, 0.f),
        0.f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) -> juce::String {
            auto text = contrast::pretifyValue(value, 3) + "dB";

            if (text[0] != '-')
                text = "-" + text;

            return text;
        },
        [](const juce::String& text) -> float {
            return text.getFloatValue();
        }
    );

    auto ratioParam = std::make_unique<juce::AudioParameterFloat>(
        Press::ParameterIDs::RATIO,
        "Ratio",
        juce::NormalisableRange<float>(1.f, 20.f),
        1.f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) -> juce::String {
            return "1 : " + contrast::pretifyValue(value, 2);
        },
        [](const juce::String& text) -> float {
            return text.fromLastOccurrenceOf("1 : ", false, true).getFloatValue();
        }
    );

    auto kneeParam = std::make_unique<juce::AudioParameterFloat>(
        Press::ParameterIDs::KNEE,
        "Knee",
        juce::NormalisableRange<float>(0.f, 20.f),
        0.f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) -> juce::String {
            return contrast::pretifyValue(value, 3) + "dB";
        },
        [](const juce::String& text) -> float {
            return text.getFloatValue();
        }
    );

    auto attackParam = std::make_unique<juce::AudioParameterFloat>(
        Press::ParameterIDs::ATTACK,
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
        Press::ParameterIDs::RELEASE,
        "Release",
        juce::NormalisableRange<float>(20.f, 5000.f),
        1000.f,
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
    
    auto gainParam = std::make_unique<juce::AudioParameterFloat>(
        Press::ParameterIDs::GAIN,
        "Gain",
        juce::NormalisableRange<float>(-18.f, 18.f),
        0.f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) -> juce::String {
            return contrast::pretifyValue(value, 3) + "dB";
        },
        [](const juce::String& text) -> float {
            return text.getFloatValue();
        }
    );

    // In this plugin we only have one, unnamed group that all of our parameters
    // we live in.
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> groups;
    groups.push_back(std::make_unique<juce::AudioProcessorParameterGroup>(
        "press", "Press", "",
        std::move(thresholdParam), std::move(ratioParam), std::move(kneeParam),
        std::move(attackParam), std::move(releaseParam), std::move(gainParam)
    ));

    return { groups.begin(), groups.end() };
}

juce::ValueTree PressProcessor::createDefaultProperties() const
{
    juce::ValueTree tree("ADDITIONAL_PROPERTIES");
    return tree;
}

void PressProcessor::presetChoiceChanged(int presetIndex)
{
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
}

void PressProcessor::updateCompressors()
{
    for (auto& compressor : compressors)
    {
        jassert(compressor != nullptr);

        compressor->setThreshold (threshold);
        compressor->setRatio     (ratio);
        compressor->setKnee      (knee);
        compressor->setAttack    (attack);
        compressor->setRelease   (release);
        compressor->setMakeupGain(gain);
    }
}

//======================================================================================================================
juce::AudioProcessorEditor* PressProcessor::createEditor()
{
    return new PressEditor(*this);
}

//======================================================================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PressProcessor();
}
