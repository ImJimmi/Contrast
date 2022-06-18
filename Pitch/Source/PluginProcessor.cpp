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
void PluginProcessor::prepareToPlay(double sampleRate, int blockSize)
{
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

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
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
    const auto numChannels = juce::jmax(getTotalNumInputChannels(), getTotalNumOutputChannels());

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
    auto newSemitones = static_cast<float>(semitones);
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
