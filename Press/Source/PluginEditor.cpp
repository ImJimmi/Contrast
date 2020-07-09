#include "PluginEditor.h"

//======================================================================================================================
PluginEditor::PluginEditor(PluginProcessor& p)
    :   AudioProcessorEditor(&p),
        processor(p),
        header(PluginProcessor::presetNames, processor.getAdditionalProperty("presetIndex", 0), contrastLaF)
{
    // Tell this Component to use the custom LookAndFeel. All child Components
    // will also use it since this it our top-level component. Also need to
    // initialise the LookAndFeel with the correct colour scheme to use (white
    // on black, or black on white).
    setLookAndFeel(&contrastLaF);
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

    // Add the ratio slider and attach it to the corresponding parameter.
    addAndMakeVisible(ratioSlider);
    ratioSlider.setName("Ratio");
    ratioSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    ratioSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 25);
    ratioAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "ratio",
        ratioSlider
    ));

    // Add the knee slider and attach it to the corresponding parameter.
    addAndMakeVisible(kneeSlider);
    kneeSlider.setName("Knee");
    kneeSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    kneeSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 25);
    kneeAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "knee",
        kneeSlider
    ));

    // Add the attack slider and attach it to the corresponding parameter.
    addAndMakeVisible(attackSlider);
    attackSlider.setName("Attack");
    attackSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    attackSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 25);
    attackAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "attack",
        attackSlider
    ));

    // Add the release slider and attach it to the corresponding parameter.
    addAndMakeVisible(releaseSlider);
    releaseSlider.setName("Release");
    releaseSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    releaseSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 25);
    releaseAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "release",
        releaseSlider
    ));

    // Add the gain slider and attach it to the corresponding parameter.
    addAndMakeVisible(gainSlider);
    gainSlider.setName("Gain");
    gainSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    gainSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 25);
    gainAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "gain",
        gainSlider
    ));

    // Set the size of the UI.
    setSize(476, 368);
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
        return width + thresholdSlider.getTextBoxHeight() * 2.f;
    };

    grid.templateColumns = { TI(80_px), TI(0_px), TI(65_px), TI(65_px), TI(0_px), TI(80_px) };
    grid.templateRows = { TI(Grid::Px(heightForWidth(65))), TI(Grid::Px(heightForWidth(65))) };
    grid.templateAreas = {
        "threshold gap ratio knee gap gain",
        "threshold gap attack release gap gain"
    };

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
        GridItem(thresholdSlider)
            .withSize(90.f, heightForWidth(80.f))
            .withArea("threshold"),

        GridItem(ratioSlider)   .withArea("ratio"),
        GridItem(kneeSlider)    .withArea("knee"),
        GridItem(attackSlider)  .withArea("attack"),
        GridItem(releaseSlider) .withArea("release"),

        GridItem(gainSlider)
            .withSize(80.f, heightForWidth(80.f))
            .withArea("gain"),
    };

    // Get the bounds of the actual 'useable' area of the UI.
    bounds.reduce(contrast::LookAndFeel::defaultThickness, contrast::LookAndFeel::defaultThickness);
    bounds.setTop(bounds.getY() - contrast::LookAndFeel::defaultThickness);

    // Perform the actual layout.
    grid.performLayout(bounds);
}
