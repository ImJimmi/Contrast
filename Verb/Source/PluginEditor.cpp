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

    // Add size slider as a child and attach it to the corresponding parameter.
    addAndMakeVisible(sizeSlider);
    sizeSlider.setName("Size");
    sizeSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    sizeSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 25);
    sizeAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "size",
        sizeSlider
    ));

    // Add damping slider as a child and attach it to the corresponding
    // parameter.
    addAndMakeVisible(dampingSlider);
    dampingSlider.setName("Damping");
    dampingSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    dampingSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 65, 25);
    dampingAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "damping",
        dampingSlider
    ));

    // Add wet slider as a child and attach it to the corresponding parameter.
    addAndMakeVisible(wetSlider);
    wetSlider.setName("Wet");
    wetSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    wetSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 65, 25);
    wetAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "wet",
        wetSlider
    ));

    // Add dry slider as a child and attach it to the corresponding parameter.
    addAndMakeVisible(drySlider);
    drySlider.setName("Dry");
    drySlider.setSliderStyle(Slider::RotaryVerticalDrag);
    drySlider.setTextBoxStyle(Slider::TextBoxBelow, false, 65, 25);
    dryAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "dry",
        drySlider
    ));

    // Add width slider as a child and attach it to the corresponding parameter.
    addAndMakeVisible(widthSlider);
    widthSlider.setName("Width");
    widthSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    widthSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 65, 25);
    dryAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(
        processor.apvts,
        "width",
        widthSlider
    ));

    // Set the size of the UI.
    setSize(356, 368);
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

    grid.templateColumns =  { TI(80_px), TI(0_px), TI(65_px), TI(65_px) };
    grid.templateRows =     { TI(115_px), TI(115_px) };

    // Make sure the sliders are centered vertically and horixontally
    grid.justifyContent = Grid::JustifyContent::center;
    grid.alignContent = Grid::AlignContent::center;
    grid.alignItems = Grid::AlignItems::center;
    grid.justifyItems = Grid::JustifyItems::center;

    // Set the gap between the sliders.
    const auto gap = 20.f;
    grid.setGap(Grid::Px(gap));

    auto heightForWidth = [this](Slider& slider, float width) -> float {
        return width + slider.getTextBoxHeight() * 2.f;
    };

    // Add the sliders to the grid and specify their required sizes.
    grid.items = {
        GridItem(sizeSlider)
            .withSize(80.f, heightForWidth(sizeSlider, 80.f))
            .withArea(GridItem::Span(2), 1),

        GridItem(),

        GridItem(dampingSlider)
            .withSize(80.f, heightForWidth(dampingSlider, 65.f)),

        GridItem(widthSlider)
            .withSize(65.f, heightForWidth(widthSlider, 65.f)),

        GridItem(),

        GridItem(wetSlider)
            .withSize(65.f, heightForWidth(wetSlider, 65.f)),

        GridItem(drySlider)
            .withSize(65.f, heightForWidth(drySlider, 65.f))
    };

    // Get the bounds of the actual 'useable' area of the UI.
    bounds.reduce(contrast::LookAndFeel::defaultThickness, contrast::LookAndFeel::defaultThickness);
    bounds.setTop(bounds.getY() - contrast::LookAndFeel::defaultThickness);

    // Perform the actual layout.
    grid.performLayout(bounds);
}
