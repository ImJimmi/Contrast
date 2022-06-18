#include "GateEditor.h"

#include "../Gate.h"

//======================================================================================================================
GateEditor::GateEditor(GateProcessor& p)
    :   AudioProcessorEditor(&p),
        gateProcessor(p),
        header(p.getPresetNames(), gateProcessor.getAdditionalProperty(contrast::PropertyIDs::PRESET_INDEX, 0), contrastLaF)
{
    // Initialise our custom LookAndFeel by setting the current colour theme.
    contrastLaF.setUseWhiteAsPrimaryColour(gateProcessor.getAdditionalProperty(
        contrast::PropertyIDs::USE_WHITE_AS_PRIMARY_COLOUR,
        contrastLaF.isUsingWhiteAsPrimaryColour()
    ));

    // Add the HeaderComponent as a child. Set its std::function members so we
    // can handle events that happen in the header.
    addAndMakeVisible(header);
    header.onPresetIndexChanged = [this](const juce::String& presetName) {
        gateProcessor.setCurrentPreset(presetName);
    };
    header.onContrastChanged = [this]() {
        gateProcessor.setAdditionalProperty(
            contrast::PropertyIDs::USE_WHITE_AS_PRIMARY_COLOUR,
            contrastLaF.isUsingWhiteAsPrimaryColour()
        );
    };

    // Initialise the sliders.
    contrast::initialiseSlider(*this, gateProcessor.getAPVTS(), thresholdSlider, thresholdAttachment,
                               "Threshold", Gate::ParameterIDs::THRESHOLD);
    contrast::initialiseSlider(*this, gateProcessor.getAPVTS(), attackSlider,    attackAttachment,
                               "Attack",    Gate::ParameterIDs::ATTACK);
    contrast::initialiseSlider(*this, gateProcessor.getAPVTS(), releaseSlider,   releaseAttachment,
                               "Release",   Gate::ParameterIDs::RELEASE);

    // Set the size of our UI.
    setSize(356, 248);

    // Tell this Component to use the custom LookAndFeel. All child Components
    // will also use it since this it our top-level component. Also need to
    // initialise the LookAndFeel with the correct colour scheme to use (white
    // on black, or black on white).
    setLookAndFeel(&contrastLaF);
}

GateEditor::~GateEditor()
{
    // Make sure to set the look and feel to nullptr before the window closes
    // to prevent memory leaks.
    setLookAndFeel(nullptr);
}

//======================================================================================================================
void GateEditor::paint(juce::Graphics& g)
{
    // Paint the background using a custom LaF method.
    contrastLaF.drawPluginBackground(g, *this);
}

void GateEditor::resized()
{
    auto bounds = getLocalBounds();
    header.setBounds(bounds.removeFromTop(35));

    // Use a grid layout for the sliders with 3 columns and 1 row.
    juce::Grid grid;
    using TI = juce::Grid::TrackInfo;
    using Px = juce::Grid::Px;

    // Use a dummy column with 0 width between the threshold and attack sliders
    // so the gap is twice as wide.
    grid.templateColumns = {
        TI(Px(contrast::sliderWidthLarge<int>)),
        TI(Px(0)),
        TI(Px(contrast::sliderWidthSmall<int>)),
        TI(Px(contrast::sliderWidthSmall<int>))
    };
    grid.templateRows =    { TI(juce::Grid::Fr(1)) };

    // Make sure the sliders are centered vertically and horizontally
    grid.justifyContent = juce::Grid::JustifyContent::center;
    grid.alignItems     = juce::Grid::AlignItems::center;
    grid.justifyItems   = juce::Grid::JustifyItems::center;

    // Set the gap between the sliders.
    grid.columnGap = Px(contrast::widgetGap<int>);

    // Add the sliders to the grid and specify their required sizes.
    grid.items = {
        juce::GridItem(thresholdSlider)
            .withSize(contrast::sliderWidthLarge<float> * 1.2f,
                      contrast::getRecommendedSliderHeightForWidth(thresholdSlider, contrast::sliderWidthLarge<float>)),

        juce::GridItem(), // Dummy item with 0 width between threshold and attack

        juce::GridItem(attackSlider)
            .withSize(contrast::sliderWidthSmall<float>,
                      contrast::getRecommendedSliderHeightForWidth(attackSlider, contrast::sliderWidthSmall<float>)),

        juce::GridItem(releaseSlider)
            .withSize(contrast::sliderWidthSmall<float>,
                      contrast::getRecommendedSliderHeightForWidth(releaseSlider, contrast::sliderWidthSmall<float>)),
    };

    // Get the bounds of the actual 'useable' area of the UI.
    bounds.reduce(contrast::defaultThickness<int>, contrast::defaultThickness<int>);
    bounds.setTop(bounds.getY() - contrast::defaultThickness<int>);

    // Perform the actual layout.
    grid.performLayout(bounds);
}
