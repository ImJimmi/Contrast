#include "PluginEditor.h"

//======================================================================================================================
PluginEditor::PluginEditor(PluginProcessor& p)
    :   AudioProcessorEditor(&p),
        processor(p),
        header(PluginProcessor::presetNames, processor.getAdditionalProperty("presetIndex", 0), contrastLaF)
{
    contrastLaF.setUseWhiteAsPrimaryColour(processor.getAdditionalProperty("useWhiteAsPrimaryColour",
                                                                           contrastLaF.isUsingWhiteAsPrimaryColour()));

    // Add the HeaderComponent as a child. Set its std::function members so we
    // can handle events that happen in the header.
    addAndMakeVisible(header);
    header.onPresetIndexChanged = [this](int i) {
        processor.setCurrentPreset(i);
    };
    header.onContrastChanged = [this]() {
        processor.setAdditionalProperty("useWhiteAsPrimaryColour", contrastLaF.isUsingWhiteAsPrimaryColour());
    };

    // Add the threshold slider and attach it to the corresponding parameter.
    addAndMakeVisible(thresholdSlider);
    thresholdSlider.setName("Threshold");
    thresholdSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    thresholdSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 25);
    thresholdAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "threshold",
        thresholdSlider
    ));

    // Add the attack slider and attach it to the corresponding parameter.
    addAndMakeVisible(attackSlider);
    attackSlider.setName("Attack");
    attackSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    attackSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 65, 25);
    attackAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "attack",
        attackSlider
    ));

    // Add the release slider and attach it to the corresponding parameter.
    addAndMakeVisible(releaseSlider);
    releaseSlider.setName("Release");
    releaseSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    releaseSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 65, 25);
    releaseAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "release",
        releaseSlider
    ));

    // Set the size of our UI.
    setSize(356, 248);

    // Tell this Component to use the custom LookAndFeel. All child Components
    // will also use it since this it our top-level component. Also need to
    // initialise the LookAndFeel with the correct colour scheme to use (white
    // on black, or black on white).
    setLookAndFeel(&contrastLaF);
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

    // Use a grid layout for the sliders with 3 columns and 1 row.
    Grid grid;
    using TI = Grid::TrackInfo;

    // Use a dummy column with 0 width between the threshold and attack sliders
    // so the gap is twice as wide.
    grid.templateColumns =  { TI(80_px), TI(0_px), TI(65_px), TI(65_px) };
    grid.templateRows =     { TI(1_fr) };

    // Make sure the sliders are centered vertically and horixontally
    grid.justifyContent = Grid::JustifyContent::center;
    grid.alignItems = Grid::AlignItems::center;
    grid.justifyItems = Grid::JustifyItems::center;

    // Set the gap between the sliders.
    const auto gap = 20.f;
    grid.columnGap = Grid::Px(gap);

    auto heightForWidth = [this](Slider& slider, float width) -> float {
        return width + slider.getTextBoxHeight() * 2.f;
    };

    // Add the sliders to the grid and specify their required sizes.
    grid.items = {
        GridItem(thresholdSlider)
            .withSize(90.f, heightForWidth(thresholdSlider, 80.f)),

        GridItem(), // Dummy item with 0 width between threshold and attack

        GridItem(attackSlider)
            .withSize(65.f, heightForWidth(attackSlider,    65.f)),

        GridItem(releaseSlider)
            .withSize(65.f, heightForWidth(releaseSlider,   65.f))
    };

    // Get the bounds of the actual 'useable' area of the UI.
    bounds.reduce(contrast::LookAndFeel::defaultThickness, contrast::LookAndFeel::defaultThickness);
    bounds.setTop(bounds.getY() - contrast::LookAndFeel::defaultThickness);

    // Perform the actual layout.
    grid.performLayout(bounds);
}
