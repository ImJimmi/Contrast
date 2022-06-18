#include "PluginEditor.h"

//======================================================================================================================
PluginEditor::PluginEditor(PluginProcessor& p)
    :   AudioProcessorEditor(&p),
        pitchProcessor(p),
        header(p.getPresetNames(), pitchProcessor.getAdditionalProperty(contrast::PropertyIDs::PRESET_INDEX, 0), contrastLaF)
{
    // Tell this Component to use the custom LookAndFeel. All child Components
    // will also use it since this it our top-level component. Also need to
    // initialise the LookAndFeel with the correct colour scheme to use (white
    // on black, or black on white).
    setLookAndFeel(&contrastLaF);
    contrastLaF.setUseWhiteAsPrimaryColour(pitchProcessor.getAdditionalProperty("useWhiteAsPrimaryColour",
                                                                                contrastLaF.isUsingWhiteAsPrimaryColour()));

    // Add the HeaderComponent as a child. Set its std::function members so we
    // can handle events that happen in the header.
    addAndMakeVisible(header);
    header.onPresetIndexChanged = [this](const juce::String& presetName) {
        pitchProcessor.setCurrentPreset(presetName);
    };
    header.onContrastChanged = [this]() {
        pitchProcessor.setAdditionalProperty(
            contrast::PropertyIDs::USE_WHITE_AS_PRIMARY_COLOUR,
            contrastLaF.isUsingWhiteAsPrimaryColour()
        );
    };

    contrast::initialiseSlider(*this, pitchProcessor.getAPVTS(), semitonesSlider, semitonesAttachment,
                               "Semitones", "semitones");
    contrast::initialiseSlider(*this, pitchProcessor.getAPVTS(), centsSlider, centsAttachment,
                               "Cents", "cents");
    contrast::initialiseSlider(*this, pitchProcessor.getAPVTS(), mixSlider, mixAttachment,
                               "Mix", "mix");

    // Set the size of the UI.
    setSize(356, 248);
}

PluginEditor::~PluginEditor()
{
    // Make sure to set the look and feel to nullptr before the window closes
    // to prevent memory leaks.
    setLookAndFeel(nullptr);
}

//======================================================================================================================
void PluginEditor::paint(Graphics& g)
{
    // Paint the background using a custom LaF method.
    contrastLaF.drawPluginBackground(g, *this);
}

void PluginEditor::resized()
{
    auto bounds = getLocalBounds();
    header.setBounds(bounds.removeFromTop(35));

    Grid grid;
    using TI = Grid::TrackInfo;

    auto heightForWidth = [this](float width) -> float {
        // Assumes all of the Sliders' text boxes are the same height.
        return width + semitonesSlider.getTextBoxHeight() * 2.f;
    };

    // Use a dummy column with 0 width between the semitones and cents sliders
    // so the gap is twice as wide.
    grid.templateColumns = { TI(80_px), TI(0_px), TI(65_px), TI(65_px) };
    grid.templateRows = { TI(1_fr) };

    // Make sure the sliders are centered vertically and horixontally
    grid.justifyContent = Grid::JustifyContent::center;
    grid.alignContent = Grid::AlignContent::center;
    grid.alignItems = Grid::AlignItems::center;
    grid.justifyItems = Grid::JustifyItems::center;

    // Set the gap between the sliders.
    const auto gap = 20.f;
    grid.setGap(Grid::Px(gap));

    // Add the sliders to the grid and specify their required sizes.
    grid.items = {
        GridItem(semitonesSlider)
            .withSize(90.f, heightForWidth(80.f)),

        GridItem(), // Dummy item with 0 width.

        GridItem(centsSlider)
            .withHeight(heightForWidth(65.f)),

        GridItem(mixSlider)
            .withHeight(heightForWidth(65.f))
    };

    // Get the bounds of the actual 'useable' area of the UI.
    bounds.reduce(contrast::defaultThickness<int>, contrast::defaultThickness<int>);
    bounds.setTop(bounds.getY() - contrast::defaultThickness<int>);

    // Perform the actual layout.
    grid.performLayout(bounds);
}
