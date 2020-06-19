#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    using namespace juce;

    //==================================================================================================================
    /** This Component should be displayed at the top of all Contrast plugins.
        It displays the plugins's name, a list of available presets, and a
        button to toggle between the two contrast modes (white on black or black
        on white).
    */
    class HeaderComponent   :   public Component
    {
    public:
        //==============================================================================================================
        /** To initialise the presets ComboBox, the list of preset names must
            be given here. The index of the initial preset to display is also
            required.
        */
        HeaderComponent(const StringArray& presetNames, int initialPresetIndex, contrast::LookAndFeel& laf)
            :   contrastLaF(laf)
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
            presetsBox.setJustificationType(Justification::centred);
            presetsBox.getProperties().set("showArrow", false);
            presetsBox.getProperties().set("invertColours", true);
            presetsBox.getProperties().set("showOutline", false);
            presetsBox.addItemList(presetNames, 1);
            presetsBox.setSelectedId(initialPresetIndex + 1, dontSendNotification);
            presetsBox.onChange = [this]() {
                if (onPresetIndexChanged)
                    onPresetIndexChanged(presetsBox.getSelectedId() - 1);
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
            contrastButton.onClick = [this]() {
                contrastButtonClicked();

                if (onContrastChanged)
                    onContrastChanged();
            };
        }

        //==============================================================================================================
        void paint(Graphics& g) override
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

            previousPresetButton.setBounds(bounds.removeFromLeft(bounds.getHeight()));
            nextPresetButton.setBounds(bounds.removeFromRight(bounds.getHeight()));

            presetsBox.setBounds(bounds);

            // Make sure to update the widgets so they're displayed properly.
            // These can't be initialised in the constructor because the custom
            // LookAndFeel won't have been set yet.
            updateContrastButtonImage();
            updateColours();
            updateArrowImages();
        }

        //==============================================================================================================
        // Called when the contrast button is clicked.
        // The LookAndFeel will have been notified about the change before this
        // is called.
        std::function<void(void)> onContrastChanged = nullptr;

        // Called when a new preset is selected from the ComboBox.
        std::function<void(int index)> onPresetIndexChanged = nullptr;

    private:
        //==============================================================================================================
        /** Updates the image used for the contrast button. */
        void updateContrastButtonImage()
        {
            auto img = contrastLaF.createContrastButtonImage(contrastButton.getWidth(), contrastButton.getHeight());
            contrastButton.setImages(false, false, true, img, 1.f, {}, img, 1.f, {}, img, 1.f, {});
        }

        /** Updates the images used for the previous and next preset buttons. */
        void updateArrowImages()
        {
            auto bounds = previousPresetButton.getLocalBounds().toFloat().withSizeKeepingCentre(11.f, 9.f);

            Path arrow;
            arrow.startNewSubPath(bounds.getBottomLeft());
            arrow.lineTo(bounds.getCentreX(), bounds.getY());
            arrow.lineTo(bounds.getBottomRight());
            arrow.closeSubPath();

            {
                Image img(Image::ARGB, previousPresetButton.getWidth(), previousPresetButton.getHeight(), true);
                Graphics g(img);

                g.setColour(presetsBox.findColour(ComboBox::textColourId));
                g.fillPath(arrow, AffineTransform::rotation(-MathConstants<float>::pi / 2.f,
                    bounds.getCentreX(), bounds.getCentreY()));

                previousPresetButton.setImages(false, false, true, img, 1.f, {}, img, 1.f, {}, img, 1.f, {});
            }

            {
                Image img(Image::ARGB, nextPresetButton.getWidth(), nextPresetButton.getHeight(), true);
                Graphics g(img);

                g.setColour(presetsBox.findColour(ComboBox::textColourId));
                g.fillPath(arrow, AffineTransform::rotation(MathConstants<float>::pi / 2.f,
                                                            bounds.getCentreX(), bounds.getCentreY()));

                nextPresetButton.setImages(false, false, true, img, 1.f, {}, img, 1.f, {}, img, 1.f, {});
            }
        }

        /** Updates the colours of the presets box. */
        void updateColours()
        {
            const auto primary = contrastLaF.findColour(contrast::LookAndFeel::primaryColourId);
            const auto secondary = contrastLaF.findColour(contrast::LookAndFeel::secondaryColourId);

            presetsBox.setColour(ComboBox::backgroundColourId,  primary);
            presetsBox.setColour(ComboBox::textColourId,        secondary);
        }

        /** Called when the contrast button is clicked and instructs the custom
            LookAndFeel to change its theme.
        */
        void contrastButtonClicked()
        {
            contrastLaF.setUseWhiteAsPrimaryColour(!contrastLaF.isUsingWhiteAsPrimaryColour());

            updateContrastButtonImage();
            updateColours();
            updateArrowImages();
            getTopLevelComponent()->repaint();
        }

        //==============================================================================================================
        contrast::LookAndFeel& contrastLaF;

        // Changes to the previous preset in the list.
        ImageButton previousPresetButton;

        // Displays a list of available presets for the plugin.
        ComboBox presetsBox;

        // Changes to the next preset in the list.
        ImageButton nextPresetButton;

        // Used to toggle between the black on white, and white on black themes.
        ImageButton contrastButton;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderComponent)
    };
}   // namespace contrast
