#include "PressEditor.h"

#include "../Press.h"

//======================================================================================================================
PressEditor::PressEditor(PressProcessor& p)
    :   AudioProcessorEditor(&p),
        processor(p),
        header(p.getPresetNames(), processor.getAdditionalProperty(contrast::PropertyIDs::PRESET_INDEX, 0), contrastLaF)
{
    // Tell this Component to use the custom LookAndFeel. All child Components
    // will also use it since this it our top-level component. Also need to
    // initialise the LookAndFeel with the correct colour scheme to use (white
    // on black, or black on white).
    setLookAndFeel(&contrastLaF);
    contrastLaF.setUseWhiteAsPrimaryColour(
        processor.getAdditionalProperty(contrast::PropertyIDs::USE_WHITE_AS_PRIMARY_COLOUR,
                                        contrastLaF.isUsingWhiteAsPrimaryColour())
    );

    // Add the HeaderComponent as a child. Set its std::function members so we
    // can handle events that happen in the header.
    addAndMakeVisible(header);
    header.onPresetIndexChanged = [this](const juce::String& presetName) {
        processor.setCurrentPreset(presetName);
    };
    header.onContrastChanged = [this]() {
        processor.setAdditionalProperty("useWhiteAsPrimaryColour", contrastLaF.isUsingWhiteAsPrimaryColour());
    };

    // Intialise the sliders.
    contrast::initialiseSlider(*this, processor.getAPVTS(), thresholdSlider, thresholdAttachment,
                               "Threshold", Press::ParameterIDs::THRESHOLD);
    contrast::initialiseSlider(*this, processor.getAPVTS(), ratioSlider,     ratioAttachment,
                               "Ratio",     Press::ParameterIDs::RATIO);
    contrast::initialiseSlider(*this, processor.getAPVTS(), kneeSlider,      kneeAttachment,
                               "Knee",      Press::ParameterIDs::KNEE);
    contrast::initialiseSlider(*this, processor.getAPVTS(), attackSlider,    attackAttachment,
                               "Attack",    Press::ParameterIDs::ATTACK);
    contrast::initialiseSlider(*this, processor.getAPVTS(), releaseSlider,   releaseAttachment,
                               "Release",   Press::ParameterIDs::RELEASE);
    contrast::initialiseSlider(*this, processor.getAPVTS(), gainSlider,      gainAttachment,
                               "Gain",      Press::ParameterIDs::GAIN);

    // Set the size of the UI.
    setSize(476, 368);
}

PressEditor::~PressEditor()
{
    // Make sure to set the look and feel to nullptr before the window closes
    // to prevent memory leaks.
    setLookAndFeel(nullptr);
}

//======================================================================================================================
void PressEditor::paint(juce::Graphics& g)
{
    // Paint the background using a custom LaF method.
    contrastLaF.drawPluginBackground(g, *this);
}

void PressEditor::resized()
{
    auto bounds = getLocalBounds();
    header.setBounds(bounds.removeFromTop(header.getStandardHeight<int>()));

    juce::Grid grid;
    using TI = juce::Grid::TrackInfo;
    using Px = juce::Grid::Px;

    grid.templateColumns = {
        TI(Px(contrast::sliderWidthLarge<int>)),
        TI(Px(0)),
        TI(Px(contrast::sliderWidthSmall<int>)),
        TI(Px(contrast::sliderWidthSmall<int>)),
        TI(Px(0)),
        TI(Px(contrast::sliderWidthLarge<int>))
    };
    grid.templateRows = {
        TI(Px(contrast::getRecommendedSliderHeightForWidth(thresholdSlider, contrast::sliderWidthSmall<int>))),
        TI(Px(contrast::getRecommendedSliderHeightForWidth(thresholdSlider, contrast::sliderWidthSmall<int>)))
    };
    grid.templateAreas = {
        "threshold gap ratio knee gap gain",
        "threshold gap attack release gap gain"
    };

    // Make sure the sliders are centered vertically and horixontally
    grid.justifyContent = juce::Grid::JustifyContent::center;
    grid.alignContent =   juce::Grid::AlignContent::center;
    grid.alignItems =     juce::Grid::AlignItems::center;
    grid.justifyItems =   juce::Grid::JustifyItems::center;

    // Set the gap between the sliders.
    grid.setGap(Px(contrast::widgetGap<int>));

    // Add the sliders to the grid and specify their required sizes.
    grid.items = {
        juce::GridItem(thresholdSlider)
            .withSize(contrast::sliderWidthLarge<float> * 1.2f, 
                      contrast::getRecommendedSliderHeightForWidth(thresholdSlider, contrast::sliderWidthLarge<float>))
            .withArea("threshold"),

        juce::GridItem(ratioSlider)   .withArea("ratio"),
        juce::GridItem(kneeSlider)    .withArea("knee"),
        juce::GridItem(attackSlider)  .withArea("attack"),
        juce::GridItem(releaseSlider) .withArea("release"),

        juce::GridItem(gainSlider)
            .withSize(contrast::sliderWidthLarge<float> * 1.2f,
                      contrast::getRecommendedSliderHeightForWidth(gainSlider, contrast::sliderWidthLarge<float>))
            .withArea("gain"),
    };

    // Get the bounds of the actual 'useable' area of the UI.
    bounds.reduce(contrast::defaultThickness<int>, contrast::defaultThickness<int>);
    bounds.setTop(bounds.getY() - contrast::defaultThickness<int>);

    // Perform the actual layout.
    grid.performLayout(bounds);
}
