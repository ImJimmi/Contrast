#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    /** This Component should be displayed at the top of all Contrast plugins.
        It displays the plugins's name, a list of available presets, and a
        button to toggle between the two contrast modes (white on black or black
        on white).
    */
    class HeaderComponent   :   public juce::Component,
                                private contrast::LookAndFeel::PrimaryColourListener
    {
    public:
        //==============================================================================================================
        /** To initialise the presets ComboBox, the list of preset names must
            be given here. The index of the initial preset to display is also
            required.
        */
        HeaderComponent(const juce::StringArray& presetNames, int initialPresetIndex, contrast::LookAndFeel& laf)
            :   contrastLaF(laf),
                previousPresetButton("Previous", juce::DrawableButton::ImageFitted),
                nextPresetButton(    "Next",     juce::DrawableButton::ImageFitted),
                contrastButton(      "Contrast", juce::DrawableButton::ImageFitted)
        {
            // Add the previous preset button as a child and set its onClick
            // method to decrement the presetBox's index.
            addAndMakeVisible(previousPresetButton);
            previousPresetButton.onClick = [this]() {
                auto index = presetsBox.getSelectedId() - 1;

                // Wrape the index round to the last index if it's below 1.
                if (index < 1)
                    index += presetsBox.getNumItems();

                presetsBox.setSelectedId(index);
            };

            // Add the presets ComboBox as a child, add the list of presets as
            // items and initialise its selected index, and finally set its
            // onChange method to call the onPresetIndexChanged member function.
            addAndMakeVisible(presetsBox);
            presetsBox.setJustificationType(juce::Justification::centred);
            presetsBox.getProperties().set(LookAndFeel::Options::SHOW_ARROW,     false);
            presetsBox.getProperties().set(LookAndFeel::Options::INVERT_COLOURS, true);
            presetsBox.getProperties().set(LookAndFeel::Options::SHOW_OUTLINE,   false);
            presetsBox.addItemList(presetNames, 1);
            presetsBox.setSelectedId(initialPresetIndex + 1, juce::dontSendNotification);
            presetsBox.onChange = [this]() {
                if (onPresetIndexChanged)
                    onPresetIndexChanged(presetsBox.getText());
            };

            // Add the next preset button as a child and set its onClick method
            // to increment the presetBox's index.
            addAndMakeVisible(nextPresetButton);
            nextPresetButton.onClick = [this]() {
                auto index = presetsBox.getSelectedId() + 1;

                // Wrap back round to 1 if the index is behond the number of
                // available items.
                if (index > presetsBox.getNumItems())
                    index = 1;

                presetsBox.setSelectedId(index);
            };

            // Add the contrast button as a child and set its onClick method
            // to instruct the custom LookAndFeel to change theme.
            addAndMakeVisible(contrastButton);
            contrastButton.setClickingTogglesState(true);
            contrastButton.setColour(juce::DrawableButton::backgroundColourId,   {});
            contrastButton.setColour(juce::DrawableButton::backgroundOnColourId, {});
            contrastButton.onClick = [this]() {
                contrastButtonClicked();

                if (onContrastChanged)
                    onContrastChanged();
            };

            contrastLaF.addPrimaryColourListener(this);
        }

        ~HeaderComponent() override
        {
            contrastLaF.removePrimaryColourListener(this);
        }

        //==============================================================================================================
        void paint(juce::Graphics& g) override
        {
            // Have the custom LookAndFeel draw the header's background and the
            // plugin name.
            contrastLaF.drawHeaderComponentBackground(g, *this);
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            contrastButton.setBounds(bounds.removeFromRight(bounds.getHeight()).reduced(5));

            bounds.removeFromLeft(contrastLaF.getHeaderPluginNameWidth(*this));
            bounds.reduce(5, 5);

            previousPresetButton.setBounds(bounds.removeFromLeft (bounds.getHeight()));
            nextPresetButton    .setBounds(bounds.removeFromRight(bounds.getHeight()));

            presetsBox.setBounds(bounds);

            // Make sure to update the widgets so they're displayed properly.
            // These can't be initialised in the constructor because the custom
            // LookAndFeel won't have been set yet.
            updateColours();
        }

        void visibilityChanged() override
        {
            updateContrastButtonDrawables();
            updateArrowDrawables();
        }

        //==============================================================================================================
        /** Returns the standard/recommended height for this Component. */
        template <typename T>
        static constexpr T getStandardHeight()
        {
            return static_cast<T>(35);
        }

        //==============================================================================================================
        // Called when the contrast button is clicked.
        // The LookAndFeel will have been notified about the change before this
        // is called.
        std::function<void(void)> onContrastChanged = nullptr;

        // Called when a new preset is selected from the ComboBox.
        std::function<void(const juce::String&)> onPresetIndexChanged = nullptr;

    private:
        //==============================================================================================================
        void primaryColourChanged() override
        {
            contrastButton.setToggleState(contrastLaF.isUsingWhiteAsPrimaryColour(), juce::sendNotification);
        }

        //==============================================================================================================
        /** Creates the drawables used by the contrast button. */
        void updateContrastButtonDrawables()
        {
            if (contrastButton.getToggleState())
            {
                if (auto drawable = juce::Drawable::createFromSVG(*juce::parseXML(Icons::contrastIconBlack)))
                    contrastButton.setImages(drawable.get());
            }
            else
            {
                if (auto drawable = juce::Drawable::createFromSVG(*juce::parseXML(Icons::contrastIconWhite)))
                    contrastButton.setImages(drawable.get());
            }
        }

        /** Creates the drawables used by the arrow buttons. */
        void updateArrowDrawables()
        {
            if (contrastButton.getToggleState())
            {
                if (auto drawable = juce::Drawable::createFromSVG(*juce::parseXML(Icons::nextIconBlack)))
                    nextPresetButton.setImages(drawable.get());
                if (auto drawable = juce::Drawable::createFromSVG(*juce::parseXML(Icons::prevIconBlack)))
                    previousPresetButton.setImages(drawable.get());
            }
            else
            {
                if (auto drawable = juce::Drawable::createFromSVG(*juce::parseXML(Icons::nextIconWhite)))
                    nextPresetButton.setImages(drawable.get());
                if (auto drawable = juce::Drawable::createFromSVG(*juce::parseXML(Icons::prevIconWhite)))
                    previousPresetButton.setImages(drawable.get());
            }
        }

        /** Updates the colours of the presets box. */
        void updateColours()
        {
            const auto primary = contrastLaF.findColour(contrast::LookAndFeel::primaryColourId);
            const auto secondary = contrastLaF.findColour(contrast::LookAndFeel::secondaryColourId);

            presetsBox.setColour(juce::ComboBox::backgroundColourId, primary);
            presetsBox.setColour(juce::ComboBox::textColourId,       secondary);
        }

        /** Called when the contrast button is clicked and instructs the custom
            LookAndFeel to change its theme.
        */
        void contrastButtonClicked()
        {
            contrastLaF.setUseWhiteAsPrimaryColour(contrastButton.getToggleState());

            updateContrastButtonDrawables();
            updateArrowDrawables();
            updateColours();
            getTopLevelComponent()->repaint();
        }

        //==============================================================================================================
        contrast::LookAndFeel& contrastLaF;

        // Changes to the previous preset in the list.
        juce::DrawableButton previousPresetButton;

        // Displays a list of available presets for the plugin.
        juce::ComboBox presetsBox;

        // Changes to the next preset in the list.
        juce::DrawableButton nextPresetButton;

        // Used to toggle between the black on white, and white on black themes.
        juce::DrawableButton contrastButton;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderComponent)
    };
}   // namespace contrast
