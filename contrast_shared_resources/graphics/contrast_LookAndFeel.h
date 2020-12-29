#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    class LookAndFeel   :   public juce::LookAndFeel_V4
    {
    public:
        //==============================================================================================================
        enum colourIds
        {
            primaryColourId =   0xC040000,
            secondaryColourId = 0xC040001
        };

        //==============================================================================================================
        LookAndFeel();

        //==============================================================================================================
        juce::Path getTickShape(float) override;

        //==============================================================================================================
        juce::Font getLabelFont(juce::Label& label) override;
        void drawLabel(juce::Graphics& g, juce::Label& label) override;
        juce::BorderSize<int> getLabelBorderSize(juce::Label&) override;

        //==============================================================================================================
        void drawRotarySlider(juce::Graphics& g, int, int, int, int, float position,
                              const float startAngle, const float endAngle, juce::Slider& slider) override;

        //==============================================================================================================
        void fillTextEditorBackground(juce::Graphics&, int, int, juce::TextEditor&) override {}
        void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override {}

        //==============================================================================================================
        juce::Font getPopupMenuFont() override;
        void drawPopupMenuBackground(juce::Graphics&, int, int) override;
        void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>&, bool, bool, bool, bool, bool,
                               const juce::String&, const juce::String&, const juce::Drawable*,
                               const juce::Colour* const) override;
        void getIdealPopupMenuItemSize(const juce::String&, bool, int, int&, int&) override;

        //==============================================================================================================
        juce::Font getComboBoxFont(juce::ComboBox&) override;
        void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
        void positionComboBoxText(juce::ComboBox&, juce::Label&) override;

        //==============================================================================================================
        void drawPluginBackground(juce::Graphics& g, juce::Component& topLevelComponent);

        //==============================================================================================================
        int getHeaderPluginNameWidth(HeaderComponent& header);
        void drawHeaderComponentBackground(juce::Graphics& g, HeaderComponent& header);
        juce::Image createContrastButtonImage(int width, int height);

        //==============================================================================================================
        void setUseWhiteAsPrimaryColour(bool shouldUseWhiteAsPrimaryColour);
        bool isUsingWhiteAsPrimaryColour();

        struct PrimaryColourListener
        {
            virtual ~PrimaryColourListener() = default;

            virtual void primaryColourChanged() = 0;
        };

        void addPrimaryColourListener(PrimaryColourListener* newListener);
        void removePrimaryColourListener(PrimaryColourListener* listenerToRemove);

        //==============================================================================================================
        /** Contains IDs of various properties used by this LookAndFeel class.
            These should be used in a Component's properties
            (Component::getProperties()) to change its LaF.
        */
        struct Options
        {
            // Whether or not the given Component should have an outline.
            static constexpr char SHOW_OUTLINE[] = "showOutline";

            // Whether or not the given Component's colours should be inverted
            // compared to the current colour theme
            // (white-on-black or black-on-white).
            static constexpr char INVERT_COLOURS[] = "invertColours";

            // Whether or not an arrow should be shown next to the text for a
            // ComboBox.
            static constexpr char SHOW_ARROW[] = "showArrow";
        };

    private:
        //==============================================================================================================
        void updateColours();

        //==============================================================================================================
        bool useWhiteAsPrimaryColour = false;
        const juce::Font font;

        juce::ListenerList<PrimaryColourListener> primaryColourListeners;
    };
}   // namespace contrast
