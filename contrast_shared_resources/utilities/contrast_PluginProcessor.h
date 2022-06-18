#pragma once

#include <JuceHeader.h>

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    /** Base class for plug-ins in the contrast bundle.

        This base class includes an APVTS object and a virtual method required
        to initialise it. It also has a ValueTree of additional properties that
        are stored within the APVTS's state but are separate from the plug-ins
        parameters and so can't be edited through the host.
        This base class also removes a lot of the boilerplate that usually comes
        with classes derived from AudioProcessor.
    */
    class PluginProcessor   :   public juce::AudioProcessor
    {
    public:
        //==============================================================================================================
        /** Default constructor that initialises the APVTS object and the
            additional properties tree.
        */
        PluginProcessor(juce::AudioProcessorValueTreeState::ParameterLayout parameterLayout,
                        juce::ValueTree defaultProperties,
                        const BusesProperties& ioLayouts = BusesProperties().withInput ("Stereo Input",  juce::AudioChannelSet::stereo())
                                                                            .withOutput("Stereo Output", juce::AudioChannelSet::stereo()))
            :   juce::AudioProcessor(ioLayouts),
                apvts(*this, &undoManager, juce::String(JucePlugin_Name).replace(" ", "_").toUpperCase(),
                      std::move(parameterLayout)),
                additionalProperties(defaultProperties)
        {
            DBG(JucePlugin_Name << " v" << JucePlugin_VersionString);

            // Make sure there's a default value for the contrast properties.
            setAdditionalProperty(contrast::PropertyIDs::USE_WHITE_AS_PRIMARY_COLOUR, false);
            setAdditionalProperty(contrast::PropertyIDs::PRESET_INDEX,                0);
        }

        virtual ~PluginProcessor() override = default;

        //==============================================================================================================
        /** Returns true if the given channel configuration is supported by
            this plugin.

            The default implementation of this provided by
            contrast::PluginProcessor returns true if both the input and output
            buses are the same size (except in Debug builds where it always
            returns true).
        */
        virtual bool isBusesLayoutSupported(const BusesLayout& layouts) const override
        {
#if JUCE_DEBUG
            juce::ignoreUnused(layouts);
            return true;
#else
            return layouts.inputBuses.size() == layouts.outputBuses.size();
#endif
        }

        //==============================================================================================================
        virtual bool hasEditor() const override              { return true; }
        virtual const juce::String getName() const override  { return JucePlugin_Name; }
        virtual bool acceptsMidi() const override            { return false; }
        virtual bool producesMidi() const override           { return false; }
        virtual bool isMidiEffect() const override           { return false; }
        virtual double getTailLengthSeconds() const override { return 0.0; }

        virtual int getNumPrograms() override                             { return 1; }
        virtual int getCurrentProgram() override                          { return 0; }
        virtual void setCurrentProgram(int) override                      {}
        virtual const juce::String getProgramName(int) override           { return {}; }
        virtual void changeProgramName(int, const juce::String&) override {}

        /** Copies the APVTS's state to the given memory block after first
            adding the additional properties tree to the state.
        */
        virtual void getStateInformation(juce::MemoryBlock& memoryBlock) override
        {
            // Get a copy of the APVTS's state.
            auto state = apvts.copyState();

            {
                // Add the additional properties to the state.
                const juce::ScopedLock lock(propertiesMutex);
                auto existingChild = state.getChildWithName(additionalProperties.getType());

                if (existingChild.isValid())
                    existingChild = additionalProperties;
                else
                    state.addChild(additionalProperties, state.getNumChildren(), nullptr);
            }

            // Copy the state to the provided memory block.
            if (auto xml = state.createXml())
                copyXmlToBinary(*xml, memoryBlock);
        }

        /** Changes the state of the APVTS using the given data. */
        virtual void setStateInformation(const void* data, int size) override
        {
            // Parse the provided data as XML.
            if (auto xml = getXmlFromBinary(data, size))
            {
                if (xml->hasTagName(apvts.state.getType()))
                {
                    // Replace the APVTS's state with the given one.
                    auto state = juce::ValueTree::fromXml(*xml);
                    apvts.replaceState(state);

                    // Retrieve the additional properties from the state.
                    const juce::ScopedLock lock(propertiesMutex);
                    auto existingChild = state.getChildWithName(additionalProperties.getType());

                    if (existingChild.isValid())
                        additionalProperties = existingChild;
                    else
                        additionalProperties = createDefaultProperties();
                }
            }
        }

        //==============================================================================================================
        /** Changes the value of the specified property in the additional
            properties tree. If no existing property is found, create a new
            property in the tree.

            This method is thread-safe but NOT realtime safe due to the use of
            a lock.
        */
        void setAdditionalProperty(const juce::Identifier& name, const juce::var& value)
        {
            const juce::ScopedLock lock(propertiesMutex);
            additionalProperties.setProperty(name, value, nullptr);
        }

        /** Returns the value of the specified property from the additional
            properties tree. If no existing property is found, returns the
            default value provided.

            This method is thread-safe but NOT realtime safe due to the use of
            a lock.
        */
        juce::var getAdditionalProperty(const juce::Identifier& name,
                                        const juce::var& defaultValue = {}) const
        {
            const juce::ScopedLock lock(propertiesMutex);
            return additionalProperties.getProperty(name, defaultValue);
        }

        /** Returns a list of preset names available for this plug-in. */
        virtual juce::StringArray getPresetNames() const = 0;

        /** Changes the plug-ins parameters according to the given preset name.
        */
        void setCurrentPreset(const juce::String& newPresetName)
        {
            //This method should only ever be called on the message thread to
            // ensure parameters are changed safely.
            JUCE_ASSERT_MESSAGE_THREAD;

            // The given preset name isn't in the list!
            jassert(getPresetNames().contains(newPresetName));

            const auto newPresetIndex = getPresetNames().indexOf(newPresetName);
            setAdditionalProperty(PropertyIDs::PRESET_INDEX, newPresetIndex);

            presetChoiceChanged(newPresetIndex);
        }

        /** Returns a reference to the APVTS object for this plugin. */
        juce::AudioProcessorValueTreeState& getAPVTS()
        {
            return apvts;
        }

    protected:
        //==============================================================================================================
        /** Derived classes should override this to provide the set of
            parameters used by this plugin.
        */
        virtual juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() const = 0;

        /** Derived classes should override this to provide a default set of
            values for the additional properties tree. This is also where the
            name of the additional properties node should be defined (which
            properly isn't important in most cases but it's worth baring in
            mind).
        */
        virtual juce::ValueTree createDefaultProperties() const = 0;

        /** Derived classes should override this and change the plugin's
            parameters based on the given parameter index.
            The current preset index is automatically stored in the additional
            properties tree so there's no need to do it manually.
        */
        virtual void presetChoiceChanged(int newPresetIndex) = 0;

    private:
        //==============================================================================================================
        // The APVTS used by this plugin where parameters are handled.
        juce::AudioProcessorValueTreeState apvts;

        // The undo manager used by the APVTS.
        juce::UndoManager undoManager;

        // A mutex used to access the addition properties tree so it can be used
        // on multiple threads.
        juce::CriticalSection propertiesMutex;

        // The additional properties tree where non-audio properties should be
        // stored.
        juce::ValueTree additionalProperties;
    };
}
