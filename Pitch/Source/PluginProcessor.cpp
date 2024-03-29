#include "PluginProcessor.h"
#include "PluginEditor.h"

//======================================================================================================================
PluginProcessor::PluginProcessor()
    :   contrast::PluginProcessor(createParameterLayout(), createDefaultProperties()),
        semitones(*dynamic_cast<AudioParameterInt*>(  getAPVTS().getParameter("semitones"))),
        cents(    *dynamic_cast<AudioParameterFloat*>(getAPVTS().getParameter("cents"))),
        mix(      *dynamic_cast<AudioParameterFloat*>(getAPVTS().getParameter("mix")))
{
    getAPVTS().addParameterListener("semitones", this);
    getAPVTS().addParameterListener("cents",     this);
    getAPVTS().addParameterListener("mix",       this);
}

PluginProcessor::~PluginProcessor()
{
    getAPVTS().removeParameterListener("semitones",  this);
    getAPVTS().removeParameterListener("cents",      this);
    getAPVTS().removeParameterListener("mix",        this);
}

//======================================================================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int newBlockSize)
{
    // Make sure the vectors have been resized to fit the current number of
    // channels.
    numChannelsChanged();

    // Initialise the compressors
    for (auto& pitShift : pitShifters)
    {
        pitShift.reset(new contrast::PitchShifter(5024, sampleRate, static_cast<uint32>(newBlockSize)));
        pitShift->setShift(2.f);
        jassert(pitShift);
    }

    // Make sure the compressors are initialised with the current parameters
    // by faking some parameter changed calls.
    parameterChanged("semitones",   static_cast<float>(semitones.get()));
    parameterChanged("cents",       cents);
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const auto numChannels = static_cast<std::size_t>(buffer.getNumChannels());
    const auto numSamples = static_cast<std::size_t>(buffer.getNumSamples());

    jassert(pitShifters.size() == numChannels);

    for (std::size_t channel = 0; channel < numChannels; channel++)
    {
        jassert(pitShifters[channel]);

        auto channelData = buffer.getWritePointer(static_cast<int>(channel));

        for (std::size_t i = 0; i < numSamples; i++)
            channelData[i] = pitShifters[channel]->processSample(channelData[i]);
    }

    setLatencySamples(pitShifters[0]->getDelayLength() / 2);
}

void PluginProcessor::processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&)
{
}

void PluginProcessor::releaseResources()
{
    pitShifters.clear();
}

void PluginProcessor::numChannelsChanged()
{
    // Since we specified to only allow configurations with the same number of
    // input and output channels, we can use either the input or the output bus
    // to find the total number of available audio channels.
    const auto numChannels = static_cast<std::size_t>(juce::jmax(getTotalNumInputChannels(),
                                                                 getTotalNumOutputChannels()));

    pitShifters.resize(numChannels);
}

//======================================================================================================================
juce::StringArray PluginProcessor::getPresetNames() const
{
    return {
        "<DEFAULT>"
    };
}

void PluginProcessor::presetChoiceChanged(int presetIndex)
{
    auto newSemitones = static_cast<int>(semitones);
    auto newCents     = static_cast<float>(cents);
    auto newMix       = static_cast<float>(mix);

    if (presetIndex == 0)
    {
        newSemitones = 0;
        newCents = 0;
        newMix = 1.f;
    }

    semitones.beginChangeGesture();
    semitones = newSemitones;
    semitones.endChangeGesture();

    cents.beginChangeGesture();
    cents = newCents;
    cents.endChangeGesture();

    mix.beginChangeGesture();
    mix = newMix;
    mix.endChangeGesture();
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
AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout() const
{
    auto semitonesParam = std::make_unique<AudioParameterInt>(
        juce::ParameterID{
            "semitones",
            1,
        },
        "Semitones",
        -24, 24, 0,
        juce::AudioParameterIntAttributes{}
            .withStringFromValueFunction([](int value, int) -> String {
                return String(value);
            })
            .withValueFromStringFunction([](const String& text) -> int {
                return text.getIntValue();
            }));

    auto centsParam = std::make_unique<AudioParameterFloat>(
        juce::ParameterID{
            "cents",
            1,
        },
        "Cents",
        NormalisableRange<float>(-100.f, 100.f),
        0.f,
        juce::AudioParameterFloatAttributes{}
            .withStringFromValueFunction([](float value, int) -> String {
                return contrast::pretifyValue(value, 3) + "%";
            })
            .withValueFromStringFunction([](const String& text) -> float {
                return text.getFloatValue();
            }));

    auto mixParam = std::make_unique<AudioParameterFloat>(
        juce::ParameterID{
            "mix",
            1,
        },
        "Mix",
        NormalisableRange<float>(0.f, 1.f),
        1.f,
        juce::AudioParameterFloatAttributes{}
            .withStringFromValueFunction([](float value, int) -> String {
                return contrast::pretifyValue(value * 100.f, 3);
            })
            .withValueFromStringFunction([](const String& text) -> float {
                return text.getFloatValue() / 100.f;
            }));

    // In this plugin we only have one, unnamed group that all of our parameters
    // we live in.
    std::vector<std::unique_ptr<AudioProcessorParameterGroup>> groups;
    groups.push_back(std::make_unique<AudioProcessorParameterGroup>(
        "pitch", "Pitch", "",
        std::move(semitonesParam), std::move(centsParam), std::move(mixParam)
    ));

    return { groups.begin(), groups.end() };
}

juce::ValueTree PluginProcessor::createDefaultProperties() const
{
    juce::ValueTree tree("ADDITIONAL_PROPERTIES");
    return tree;
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
