#pragma once

#include <JuceHeader.h>

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    class PlugInState
    {
    public:
        //==============================================================================================================
        PluginState(juce::AudioProcessor& processor,
                    juce::AudioProcessorValueTreeState::ParameterLayout parameterLayout)
            : undoManager{ std::make_shared<juce::UndoManager>() }
            , apvts{ std::make_shared<juce::AudioProcessorValueTreeState>(processor,
                                                                          &undoManager,
                                                                          juce::String{ JucePlugin_Name }
                                                                              .replace(" ", "_")
                                                                              .toUpperCase(),
                                                                          parameterLayout) }
        {
        }

        //==============================================================================================================
        void canUndo() const
        {
            undoManager.canUndo();
        }

        void canRedo() const
        {
            undoManager.canRedo();
        }

        //==============================================================================================================
        void undo()
        {
            jassert(undoManager.undo());
        }

        void redo()
        {
            jassert(undoManager.redo());
        }

    protected:
        //==============================================================================================================
        template <typename Parameter>
        std::optional<std::reference_wrapper<Parameter>> getParameter(juce::Identifier parameterID)
        {
            static_assert(std::is_base_of<juce::RangedAudioParameter, Parameter>::value,
                          "Parameter must be derived from juce::RangedAudioParameter!");

            if (auto parameter = dynamic_cast<Parameter*>(apvts.getRawParameter(parameterID)))
                return std::make_optional<std::reference_wrapper<Parameter>>(*parameter);

            return {};
        }

    private:
        //==============================================================================================================
        std::shared_ptr<juce::UndoManager> undoManager;
        std::shared_ptr<juce::AudioProcessorValueTreeState> apvts;
    };
} // namespace contrast
