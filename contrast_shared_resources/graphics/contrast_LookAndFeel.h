#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    using namespace juce;

    //==================================================================================================================
    class LookAndFeel   :   public LookAndFeel_V4
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
        Path getTickShape(float) override;

        //==============================================================================================================
        Font getLabelFont(Label& label) override;
        void drawLabel(Graphics& g, Label& label) override;
        BorderSize<int> getLabelBorderSize(Label&) override;

        //==============================================================================================================
        void drawRotarySlider(Graphics& g, int, int, int, int, float position,
                              const float startAngle, const float endAngle, Slider& slider) override;

        //==============================================================================================================
        void fillTextEditorBackground(Graphics&, int, int, TextEditor&) override {}
        void drawTextEditorOutline(Graphics&, int, int, TextEditor&) override {}

        //==============================================================================================================
        Font getPopupMenuFont() override;
        void drawPopupMenuBackground(Graphics&, int, int) override;
        void drawPopupMenuItem(Graphics&, const Rectangle<int>&, bool, bool, bool, bool, bool, const String&,
                               const String&, const Drawable*, const Colour* const) override;
        void getIdealPopupMenuItemSize(const String&, bool, int, int&, int&) override;

        //==============================================================================================================
        Font getComboBoxFont(ComboBox&) override;
        void drawComboBox(Graphics&, int, int, bool, int, int, int, int, ComboBox&) override;
        void positionComboBoxText(ComboBox&, Label&) override;

        //==============================================================================================================
        void drawPluginBackground(Graphics& g, Component& topLevelComponent);

        //==============================================================================================================
        int getHeaderPluginNameWidth(HeaderComponent& header);
        void drawHeaderComponentBackground(Graphics& g, HeaderComponent& header);
        Image createContrastButtonImage(int width, int height);

        //==============================================================================================================
        void setUseWhiteAsPrimaryColour(bool shouldUseWhiteAsPrimaryColour);
        bool isUsingWhiteAsPrimaryColour();

        //==============================================================================================================
        static const int defaultThickness;

    private:
        //==============================================================================================================
        void updateColours();

        //==============================================================================================================
        bool useWhiteAsPrimaryColour = false;
        const Font font;
    };
}   // namespace contrast
