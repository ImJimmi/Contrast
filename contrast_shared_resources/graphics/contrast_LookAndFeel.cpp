#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    using namespace juce;

    //==================================================================================================================
    const int LookAndFeel::defaultThickness = 3;

    //==================================================================================================================
    LookAndFeel::LookAndFeel()
        :   font(Font("Arial", 18.f, 0).withTypefaceStyle("Black"))
    {
        updateColours();
    }

    //==================================================================================================================
    Path LookAndFeel::getTickShape(float size)
    {
        Rectangle<float> bounds(0.f, 0.f, size, size);
        bounds.reduce(defaultThickness, defaultThickness);

        Path tick;
        tick.startNewSubPath(bounds.getRight(), bounds.getY() + bounds.getHeight() * 0.17f);
        tick.lineTo(bounds.getX() + bounds.getWidth() * 0.36f, bounds.getY() + bounds.getHeight() * 0.77f);
        tick.lineTo(bounds.getX(), bounds.getY() + bounds.getHeight() * 0.45f);

        return tick;
    }

    //==================================================================================================================
    Font LookAndFeel::getLabelFont(Label& label)
    {
        return font.withHeight(jmin(static_cast<float>(label.getHeight()), font.getHeight()));
    }

    void LookAndFeel::drawLabel(Graphics& g, Label& label)
    {
        auto bounds = label.getLocalBounds();

        g.fillAll(findColour(Label::backgroundColourId));

        // outline
        if (label.getProperties().getWithDefault("showOutline", true))
        {
            if (label.getProperties().getWithDefault("invertColours", false))
                g.setColour(findColour(Label::outlineColourId).getBrightness() > 0.5f ? Colours::black : Colours::white);
            else
                g.setColour(findColour(Label::outlineColourId));

            g.drawRect(bounds, defaultThickness);
        }

        // text
        if (label.getProperties().getWithDefault("invertColours", false))
            g.setColour(findColour(Label::textColourId).getBrightness() > 0.5f ? Colours::black : Colours::white);
        else
            g.setColour(findColour(Label::textColourId));

        if (!label.isBeingEdited())
        {
            bounds.reduce(defaultThickness * 2, defaultThickness * 2);

            auto font = getLabelFont(label);
            g.setFont(getLabelFont(label));

            if (font.getStringWidth(label.getText()) > label.getWidth())
            {
                g.drawMultiLineText(label.getText(), bounds.getX(), bounds.getY() + static_cast<int>(font.getAscent()) - font.getHeight() * 0.15f,
                    bounds.getWidth(), label.getJustificationType(), -font.getHeight() * 0.1f);
            }
            else
            {
                g.drawText(label.getText(), label.getLocalBounds().translated(0, -1), label.getJustificationType());
            }
        }
        else
        {
            auto editor = label.getCurrentTextEditor();

            editor->setJustification(Justification::centredTop);
            editor->setTransform(AffineTransform::translation(0.f, -1.5f));
            editor->setColour(TextEditor::textColourId,               findColour(primaryColourId));
            editor->setColour(TextEditor::highlightColourId,          findColour(primaryColourId));
            editor->setColour(TextEditor::highlightedTextColourId,    findColour(secondaryColourId));
        }
    }

    BorderSize<int> LookAndFeel::getLabelBorderSize(Label&)
    {
        return BorderSize<int>(defaultThickness);
    }

    //==================================================================================================================
    void LookAndFeel::drawRotarySlider(Graphics& g, int, int, int, int, float position, float, float, Slider& slider)
    {
        auto bounds = slider.getLocalBounds().toFloat();

        // draw name
        if (slider.getName().isNotEmpty())
        {
            g.setColour(findColour(primaryColourId));
            g.setFont(font.withHeight(jmin(static_cast<float>(slider.getTextBoxHeight()), font.getHeight())));
            g.drawText(
                slider.getName().toUpperCase(),
                bounds.removeFromTop(slider.getTextBoxHeight()).toNearestInt().translated(0, -2),
                Justification::centredTop
            );
        }

        if (slider.getTextBoxPosition() != Slider::NoTextBox)
            bounds.removeFromBottom(static_cast<float>(slider.getTextBoxHeight()));

        const auto size = jmin(bounds.getWidth(), bounds.getHeight());
        bounds = bounds.withSizeKeepingCentre(size, size);

        const auto startAngle = -MathConstants<float>::pi * 0.75f;
        const auto endAngle = -startAngle;
        const auto scale = bounds.getWidth() / 60.f;

        g.setColour(findColour(primaryColourId));

        for (int i = 0; i < 13; i++)
        {
            auto angle = startAngle + (endAngle - startAngle) * i / 12.f;

            auto point = bounds.getCentre().getPointOnCircumference(bounds.getWidth() / 2.f - defaultThickness * scale, angle);
            g.fillEllipse(point.x - defaultThickness / 2.f - 0.5f, point.y - defaultThickness / 2.f - 0.5f,
                          defaultThickness + 1.f, defaultThickness + 1.f);
        }

        bounds.reduce(defaultThickness * 3.5f * scale, defaultThickness * 3.5f * scale);
        g.drawEllipse(bounds, defaultThickness);

        bounds.reduce(defaultThickness * 2.f, defaultThickness * 2.f);

        Path line;
        line.startNewSubPath(bounds.getCentreX(), bounds.getY());
        line.lineTo(bounds.getCentreX(), bounds.getY() + 10.f * scale);
        g.strokePath(
            line,
            PathStrokeType(defaultThickness, PathStrokeType::curved, PathStrokeType::rounded),
            AffineTransform::rotation(
                startAngle + (endAngle - startAngle) * position,
                bounds.getCentreX(),
                bounds.getCentreY()
            )
        );
    }

    //==================================================================================================================
    Font LookAndFeel::getPopupMenuFont()
    {
        return font;
    }

    void LookAndFeel::drawPopupMenuBackground(Graphics& g, int width, int height)
    {
        Rectangle<int> bounds(0, 0, width, height);

        g.setColour(findColour(secondaryColourId));
        g.fillRect(bounds);

        g.setColour(findColour(primaryColourId));
        g.drawRect(bounds, defaultThickness);
    }

    void LookAndFeel::drawPopupMenuItem(Graphics& g, const Rectangle<int>& bounds,
                                        bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                                        const String& text, const String& shortcutKeyText, const Drawable* icon,
                                        const Colour* const textColourToUse)
    {
        if (isSeparator)
        {
            auto r = bounds.withSizeKeepingCentre(bounds.getWidth() - defaultThickness * 6, 1);

            g.setColour(findColour(primaryColourId));
            g.fillRect(r);
        }
        else
        {
            auto r = bounds.reduced(defaultThickness * 3, defaultThickness * 2);
            auto iconBounds = r.toFloat().removeFromLeft(r.getHeight() * 0.8f);
            auto tickBounds = r.toFloat().removeFromRight(r.getHeight() * 0.8f);

            g.setColour(findColour(primaryColourId));

            if (icon != nullptr)
                icon->drawWithin(g, iconBounds, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.f);

            if (hasSubMenu)
            {
                Path p;
                p.startNewSubPath(tickBounds.getTopLeft());
                p.lineTo(tickBounds.getBottomLeft());
                p.lineTo(tickBounds.getRight(), tickBounds.getCentreY());
                p.closeSubPath();

                g.fillPath(p);
            }
            else if (isTicked)
            {
                auto tick = getTickShape(tickBounds.getHeight());
                g.strokePath(tick, PathStrokeType(defaultThickness), tick.getTransformToScaleToFit(tickBounds, true));
            }

            r.reduce(defaultThickness * 4, 0);

            auto f = getPopupMenuFont();
            f.setUnderline(isHighlighted);
            g.setFont(f);
            g.drawText(text.toUpperCase(), r, Justification::centredLeft);

            if (!isActive)
            {
                const auto width = f.getStringWidth(text.toUpperCase());
                g.fillRect(r.removeFromLeft(width).withSizeKeepingCentre(width, defaultThickness).translated(0, 1));
            }

            if (shortcutKeyText.isNotEmpty())
            {
                g.setFont(font.withHeight(font.getHeight() * 0.8f));
                g.drawText(shortcutKeyText.toUpperCase(), r, Justification::centredRight);
            }
        }
    }

    void LookAndFeel::getIdealPopupMenuItemSize(const String& text, bool isSeparator, int, int& width, int& height)
    {
        if (isSeparator)
        {
            width = 50;
            height = defaultThickness * 2 + 1;
        }
        else
        {
            auto f = getPopupMenuFont();
            height = static_cast<int>(std::ceil(f.getHeight())) + defaultThickness * 2;
            width = font.getStringWidth(text) + height * 3;
        }
    }

    //==================================================================================================================
    Font LookAndFeel::getComboBoxFont(ComboBox& box)
    {
        return font.withHeight(jmin(static_cast<float>(box.getHeight()), font.getHeight()));
    }

    void LookAndFeel::drawComboBox(Graphics& g, int, int, bool, int, int, int, int, ComboBox& box)
    {
        auto bounds = box.getLocalBounds();

        // background
        if (box.getProperties().getWithDefault("invertColours", false))
            g.setColour(findColour(ComboBox::backgroundColourId).getBrightness() > 0.5f ? Colours::black : Colours::white);
        else
            g.setColour(findColour(ComboBox::backgroundColourId));

        g.fillRect(bounds);

        // outline
        if (box.getProperties().getWithDefault("showOutline", true))
        {
            if (box.getProperties().getWithDefault("invertColours", false))
                g.setColour(findColour(ComboBox::outlineColourId).getBrightness() > 0.5f ? Colours::black : Colours::white);
            else
                g.setColour(findColour(ComboBox::outlineColourId));

            g.drawRect(bounds, defaultThickness);
        }

        if (box.getProperties().getWithDefault("showArrow", true))
        {
            auto arrowBounds = bounds.removeFromRight(jmin(bounds.getHeight(), 25)).toFloat().withSizeKeepingCentre(9, 7);

            Path arrow;
            arrow.startNewSubPath(arrowBounds.getTopLeft());
            arrow.lineTo(arrowBounds.getCentreX(), arrowBounds.getBottom());
            arrow.lineTo(arrowBounds.getTopRight());
            arrow.closeSubPath();

            if (box.getProperties().getWithDefault("invertColours", false))
                g.setColour(findColour(ComboBox::arrowColourId).getBrightness() > 0.5f ? Colours::black : Colours::white);
            else
                g.setColour(findColour(ComboBox::arrowColourId));

            g.fillPath(arrow);
        }
    }

    void LookAndFeel::positionComboBoxText(ComboBox& box, Label& label)
    {
        auto bounds = box.getLocalBounds();

        if (box.getProperties().getWithDefault("showArrow", true))
            bounds.removeFromRight(jmin(bounds.getHeight(), 25));

        if (box.getJustificationType() != Justification::centred)
            bounds.removeFromLeft(7);

        label.setBounds(bounds);
        label.setFont(getComboBoxFont(box));
        label.getProperties().set("showOutline", false);
        label.getProperties().set("invertColours", box.getProperties().getWithDefault("invertColours", false));
    }

    //==================================================================================================================
    void LookAndFeel::drawPluginBackground(Graphics& g, Component& topLevelComponent)
    {
        g.fillAll(findColour(contrast::LookAndFeel::secondaryColourId));

        g.setColour(findColour(contrast::LookAndFeel::primaryColourId));
        g.drawRect(topLevelComponent.getLocalBounds(), contrast::LookAndFeel::defaultThickness);
    }

    //==================================================================================================================
    int LookAndFeel::getHeaderPluginNameWidth(HeaderComponent& header)
    {
        const auto nameFont = font.withHeight(static_cast<float>(header.getHeight()));
        const auto name = String(JucePlugin_Name).toUpperCase();
        return nameFont.getStringWidth(name) + 17;
    }

    void LookAndFeel::drawHeaderComponentBackground(Graphics& g, HeaderComponent& header)
    {
        g.setColour(findColour(primaryColourId));
        g.fillAll();

        auto bounds = header.getLocalBounds();

        {
            const auto nameFont = font.withHeight(static_cast<float>(header.getHeight()));
            const auto name = String(JucePlugin_Name).toUpperCase();
            const auto stringWidth = nameFont.getStringWidth(name) + 10;

            bounds.removeFromLeft(7);
            const auto nameBounds = bounds.removeFromLeft(stringWidth);

            g.setColour(findColour(secondaryColourId));
            g.setFont(nameFont);
            g.drawText(name, nameBounds.translated(0, -1), Justification::centredLeft);
        }
    }

    Image LookAndFeel::createContrastButtonImage(int width, int height)
    {
        Image img(Image::ARGB, width, height, true);
        Graphics g(img);

        auto bounds = img.getBounds().reduced(defaultThickness).toFloat();

        Path path;
        path.addPieSegment(bounds, 0.f, MathConstants<float>::pi, 0.f);

        g.setColour(findColour(secondaryColourId));
        g.fillPath(path);

        path.clear();
        path.addEllipse(bounds);
        g.strokePath(path, PathStrokeType(static_cast<float>(defaultThickness)));

        return img;
    }

    //==================================================================================================================
    void LookAndFeel::setUseWhiteAsPrimaryColour(bool shouldUseWhiteAsPrimaryColour)
    {
        useWhiteAsPrimaryColour = shouldUseWhiteAsPrimaryColour;
        updateColours();
    }

    bool LookAndFeel::isUsingWhiteAsPrimaryColour()
    {
        return useWhiteAsPrimaryColour;
    }

    //==================================================================================================================
    void LookAndFeel::updateColours()
    {
        Colour primary, secondary;

        if (useWhiteAsPrimaryColour)
        {
            primary = Colours::white;
            secondary = Colours::black;
        }
        else
        {
            primary = Colours::black;
            secondary = Colours::white;
        }

        setColour(primaryColourId,      primary);
        setColour(secondaryColourId,    secondary);

        setColour(Label::backgroundColourId,    {});
        setColour(Label::outlineColourId,       primary);
        setColour(Label::textColourId,          primary);

        setColour(Slider::textBoxBackgroundColourId,    secondary);
        setColour(Slider::textBoxOutlineColourId,       primary);
        setColour(Slider::textBoxTextColourId,          primary);

        setColour(TextEditor::textColourId,             primary);
        setColour(TextEditor::highlightedTextColourId,  secondary);
        setColour(CaretComponent::caretColourId,        primary);
        setColour(PopupMenu::textColourId,              primary);

        setColour(ComboBox::backgroundColourId,     secondary);
        setColour(ComboBox::textColourId,           primary);
        setColour(ComboBox::outlineColourId,        primary);
        setColour(ComboBox::buttonColourId,         secondary);
        setColour(ComboBox::arrowColourId,          primary);
        setColour(ComboBox::focusedOutlineColourId, primary);
    }
}   // namespace contrast
