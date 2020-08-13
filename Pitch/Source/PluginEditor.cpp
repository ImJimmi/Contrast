#include "PluginEditor.h"

//======================================================================================================================
PluginEditor::PluginEditor(PluginProcessor& p)
    :   AudioProcessorEditor(&p),
        processor(p),
        header(PluginProcessor::presetNames, processor.getAdditionalProperty("presetIndex", 0))
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
    addAndMakeVisible(semitonesSlider);
    semitonesSlider.setName("Semitones");
    semitonesSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    semitonesSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 25);
    semitonesAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "semitones",
        semitonesSlider
    ));

    // Add the ratio slider and attach it to the corresponding parameter.
    addAndMakeVisible(centsSlider);
    centsSlider.setName("Cents");
    centsSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    centsSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 25);
    centsAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "cents",
        centsSlider
    ));

    // Add the ratio slider and attach it to the corresponding parameter.
    addAndMakeVisible(mixSlider);
    mixSlider.setName("Mix");
    mixSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 25);
    mixAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "mix",
        mixSlider
    ));

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
    bounds.reduce(contrast::LookAndFeel::defaultThickness, contrast::LookAndFeel::defaultThickness);
    bounds.setTop(bounds.getY() - contrast::LookAndFeel::defaultThickness);

    // Perform the actual layout.
    grid.performLayout(bounds);
}
