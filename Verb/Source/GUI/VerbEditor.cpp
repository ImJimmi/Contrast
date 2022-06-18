#include "VerbEditor.h"

#include "../Verb.h"

//======================================================================================================================
VerbEditor::VerbEditor(VerbProcessor& p)
    :   AudioProcessorEditor(&p),
        verbProcessor(p),
        header(p.getPresetNames(), verbProcessor.getAdditionalProperty(contrast::PropertyIDs::PRESET_INDEX, 0), contrastLaF)
{
    // Tell this Component to use the custom LookAndFeel. All child Components
    // will also use it since this it our top-level component. Also need to
    // initialise the LookAndFeel with the correct colour scheme to use (white
    // on black, or black on white).
    setLookAndFeel(&contrastLaF);
    contrastLaF.setUseWhiteAsPrimaryColour(
        verbProcessor.getAdditionalProperty(contrast::PropertyIDs::USE_WHITE_AS_PRIMARY_COLOUR,
                                        contrastLaF.isUsingWhiteAsPrimaryColour())
    );

    // Add the HeaderComponent as a child. Set its std::function members so we
    // can handle events that happen in the header.
    addAndMakeVisible(header);
    header.onPresetIndexChanged = [this](const juce::String& presetName) {
        verbProcessor.setCurrentPreset(presetName);
    };
    header.onContrastChanged = [this]() {
        verbProcessor.setAdditionalProperty(contrast::PropertyIDs::USE_WHITE_AS_PRIMARY_COLOUR, contrastLaF.isUsingWhiteAsPrimaryColour());
    };

    // Initialise sliders.
    contrast::initialiseSlider(*this, verbProcessor.getAPVTS(), sizeSlider,    sizeAttachment,
                               "Size",    Verb::ParameterIDs::SIZE);
    contrast::initialiseSlider(*this, verbProcessor.getAPVTS(), dampingSlider, dampingAttachment,
                               "Damping", Verb::ParameterIDs::DAMPING);
    contrast::initialiseSlider(*this, verbProcessor.getAPVTS(), wetSlider,     wetAttachment,
                               "Wet",     Verb::ParameterIDs::WET);
    contrast::initialiseSlider(*this, verbProcessor.getAPVTS(), drySlider,     dryAttachment,
                               "Dry",     Verb::ParameterIDs::DRY);
    contrast::initialiseSlider(*this, verbProcessor.getAPVTS(), widthSlider,   widthAttachment,
                               "Width",   Verb::ParameterIDs::WIDTH);

    // Set the size of the UI.
    setSize(356, 368);
}

VerbEditor::~VerbEditor()
{
    // Make sure to set the look and feel to nullptr before the window closes
    // to prevent memory leaks.
    setLookAndFeel(nullptr);
}

//======================================================================================================================
void VerbEditor::paint(juce::Graphics& g)
{
    // Paint the background using a custom LaF method.
    contrastLaF.drawPluginBackground(g, *this);
}

void VerbEditor::resized()
{
    auto bounds = getLocalBounds();
    header.setBounds(bounds.removeFromTop(35));

    juce::Grid grid;
    using TI = juce::Grid::TrackInfo;
    using Px = juce::Grid::Px;

    grid.templateColumns =  { TI(Px(80)), TI(Px(0)), TI(Px(65)), TI(Px(65)) };
    grid.templateRows =     { TI(Px(115)), TI(Px(115)) };

    // Make sure the sliders are centered vertically and horixontally
    grid.justifyContent = juce::Grid::JustifyContent::center;
    grid.alignContent   = juce::Grid::AlignContent::center;
    grid.alignItems     = juce::Grid::AlignItems::center;
    grid.justifyItems   = juce::Grid::JustifyItems::center;

    // Set the gap between the sliders.
    const auto gap = 20.f;
    grid.setGap(Px(gap));

    auto heightForWidth = [](juce::Slider& slider, float width) -> float {
        return width + slider.getTextBoxHeight() * 2.f;
    };

    // Add the sliders to the grid and specify their required sizes.
    grid.items = {
        juce::GridItem(sizeSlider)
            .withSize(80.f, heightForWidth(sizeSlider, 80.f))
            .withArea(juce::GridItem::Span(2), 1),

        juce::GridItem(),

        juce::GridItem(dampingSlider)
            .withSize(80.f, heightForWidth(dampingSlider, 65.f)),

        juce::GridItem(widthSlider)
            .withSize(65.f, heightForWidth(widthSlider, 65.f)),

        juce::GridItem(),

        juce::GridItem(wetSlider)
            .withSize(65.f, heightForWidth(wetSlider, 65.f)),

        juce::GridItem(drySlider)
            .withSize(65.f, heightForWidth(drySlider, 65.f))
    };

    // Get the bounds of the actual 'useable' area of the UI.
    bounds.reduce(contrast::defaultThickness<int>, contrast::defaultThickness<int>);
    bounds.setTop(bounds.getY() - contrast::defaultThickness<int>);

    // Perform the actual layout.
    grid.performLayout(bounds);
}
