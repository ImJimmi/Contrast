#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    LookAndFeel::LookAndFeel()
        :   font(juce::Font("Arial", 18.f, 0).withTypefaceStyle("Black"))
    {
        updateColours();
    }

    //==================================================================================================================
    juce::Path LookAndFeel::getTickShape(float size)
    {
        juce::Rectangle<float> bounds(0.f, 0.f, size, size);
        bounds.reduce(defaultThickness<float>, defaultThickness<float>);

        juce::Path tick;
        tick.startNewSubPath(bounds.getRight(), bounds.getY() + bounds.getHeight() * 0.17f);
        tick.lineTo(bounds.getX() + bounds.getWidth() * 0.36f, bounds.getY() + bounds.getHeight() * 0.77f);
        tick.lineTo(bounds.getX(), bounds.getY() + bounds.getHeight() * 0.45f);

        return tick;
    }

    //==================================================================================================================
    juce::Font LookAndFeel::getLabelFont(juce::Label& label)
    {
        return font.withHeight(juce::jmin(static_cast<float>(label.getHeight()), font.getHeight()));
    }

    void LookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
    {
        auto bounds = label.getLocalBounds();

        g.fillAll(findColour(juce::Label::backgroundColourId));

        // Outline.
        if (label.getProperties().getWithDefault(Options::SHOW_OUTLINE, true))
        {
            if (label.getProperties().getWithDefault(Options::INVERT_COLOURS, false))
                g.setColour(findColour(juce::Label::outlineColourId).getBrightness() > 0.5f ? juce::Colours::black : juce::Colours::white);
            else
                g.setColour(findColour(juce::Label::outlineColourId));

            g.drawRect(bounds, defaultThickness<int>);
        }

        // Text.
        if (label.getProperties().getWithDefault(Options::INVERT_COLOURS, false))
            g.setColour(findColour(juce::Label::textColourId).getBrightness() > 0.5f ? juce::Colours::black : juce::Colours::white);
        else
            g.setColour(findColour(juce::Label::textColourId));

        if (!label.isBeingEdited())
        {
            bounds.reduce(defaultThickness<int> * 2, defaultThickness<int> * 2);

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

            editor->setJustification(juce::Justification::centredTop);
            editor->setTransform(juce::AffineTransform::translation(0.f, -1.5f));
            editor->setColour(juce::TextEditor::textColourId,            findColour(primaryColourId));
            editor->setColour(juce::TextEditor::highlightColourId,       findColour(primaryColourId));
            editor->setColour(juce::TextEditor::highlightedTextColourId, findColour(secondaryColourId));
        }
    }

    juce::BorderSize<int> LookAndFeel::getLabelBorderSize(juce::Label&)
    {
        return juce::BorderSize<int>(defaultThickness<int>);
    }

    //==================================================================================================================
    void LookAndFeel::drawRotarySlider(juce::Graphics& g, int, int, int, int, float position, float, float,
                                       juce::Slider& slider)
    {
        auto bounds = slider.getLocalBounds().toFloat();

        // draw name
        if (slider.getName().isNotEmpty())
        {
            g.setColour(findColour(primaryColourId));
            g.setFont(font.withHeight(juce::jmin(static_cast<float>(slider.getTextBoxHeight()), font.getHeight())));
            g.drawText(
                slider.getName().toUpperCase(),
                bounds.removeFromTop(slider.getTextBoxHeight()).toNearestInt().translated(0, -2),
                juce::Justification::centredTop
            );
        }

        if (slider.getTextBoxPosition() != juce::Slider::NoTextBox)
            bounds.removeFromBottom(static_cast<float>(slider.getTextBoxHeight()));

        const auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
        bounds = bounds.withSizeKeepingCentre(size, size);

        const auto startAngle = -juce::MathConstants<float>::pi * 0.75f;
        const auto endAngle = -startAngle;
        const auto scale = bounds.getWidth() / 60.f;

        g.setColour(findColour(primaryColourId));

        for (int i = 0; i < 13; i++)
        {
            auto angle = startAngle + (endAngle - startAngle) * i / 12.f;
            auto point = bounds.getCentre().getPointOnCircumference(
                bounds.getWidth() / 2.f - defaultThickness<float> * scale,
                angle
            );
            g.fillEllipse(point.x - defaultThickness<float> / 2.f - 0.5f,
                          point.y - defaultThickness<float> / 2.f - 0.5f,
                          defaultThickness<float> + 1.f,
                          defaultThickness<float> + 1.f);
        }

        bounds.reduce(defaultThickness<float> * 3.5f * scale, defaultThickness<float> * 3.5f * scale);
        g.drawEllipse(bounds, defaultThickness<float>);

        bounds.reduce(defaultThickness<float> * 2.f, defaultThickness<float> * 2.f);

        juce::Path line;
        line.startNewSubPath(bounds.getCentreX(), bounds.getY());
        line.lineTo(bounds.getCentreX(), bounds.getY() + 10.f * scale);
        g.strokePath(
            line,
            juce::PathStrokeType(defaultThickness<float>, juce::PathStrokeType::curved, juce::PathStrokeType::rounded),
            juce::AffineTransform::rotation(
                startAngle + (endAngle - startAngle) * position,
                bounds.getCentreX(),
                bounds.getCentreY()
            )
        );
    }

    //==================================================================================================================
    juce::Font LookAndFeel::getPopupMenuFont()
    {
        return font;
    }

    void LookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
    {
        juce::Rectangle<int> bounds(0, 0, width, height);

        g.setColour(findColour(secondaryColourId));
        g.fillRect(bounds);

        g.setColour(findColour(primaryColourId));
        g.drawRect(bounds, defaultThickness<float>);
    }

    void LookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                        bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                                        bool hasSubMenu,
                                        const juce::String& text, const juce::String& shortcutKeyText,
                                        const juce::Drawable* icon,
                                        const juce::Colour* const textColourToUse)
    {
        if (isSeparator)
        {
            auto r = bounds.withSizeKeepingCentre(bounds.getWidth() - defaultThickness<int> * 6, 1);

            g.setColour(findColour(primaryColourId));
            g.fillRect(r);
        }
        else
        {
            auto r = bounds.reduced(defaultThickness<int> * 3, defaultThickness<int> * 2);
            auto iconBounds = r.toFloat().removeFromLeft(r.getHeight() * 0.8f);
            auto tickBounds = r.toFloat().removeFromRight(r.getHeight() * 0.8f);

            g.setColour(findColour(primaryColourId));

            if (icon != nullptr)
                icon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.f);

            if (hasSubMenu)
            {
                juce::Path p;
                p.startNewSubPath(tickBounds.getTopLeft());
                p.lineTo(tickBounds.getBottomLeft());
                p.lineTo(tickBounds.getRight(), tickBounds.getCentreY());
                p.closeSubPath();

                g.fillPath(p);
            }
            else if (isTicked)
            {
                auto tick = getTickShape(tickBounds.getHeight());
                g.strokePath(tick, juce::PathStrokeType(defaultThickness<float>), tick.getTransformToScaleToFit(tickBounds, true));
            }

            r.reduce(defaultThickness<int> * 4, 0);

            auto f = getPopupMenuFont();
            f.setUnderline(isHighlighted);
            g.setFont(f);
            g.drawText(text.toUpperCase(), r, juce::Justification::centredLeft);

            if (!isActive)
            {
                const auto width = f.getStringWidth(text.toUpperCase());
                g.fillRect(r.removeFromLeft(width).withSizeKeepingCentre(width, defaultThickness<int>).translated(0, 1));
            }

            if (shortcutKeyText.isNotEmpty())
            {
                g.setFont(font.withHeight(font.getHeight() * 0.8f));
                g.drawText(shortcutKeyText.toUpperCase(), r, juce::Justification::centredRight);
            }
        }
    }

    void LookAndFeel::getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int, int& width, int& height)
    {
        if (isSeparator)
        {
            width = 50;
            height = defaultThickness<int> * 2 + 1;
        }
        else
        {
            auto f = getPopupMenuFont();
            height = static_cast<int>(std::ceil(f.getHeight())) + defaultThickness<int> * 2;
            width = font.getStringWidth(text) + height * 3;
        }
    }

    //==================================================================================================================
    juce::Font LookAndFeel::getComboBoxFont(juce::ComboBox& box)
    {
        return font.withHeight(juce::jmin(static_cast<float>(box.getHeight()), font.getHeight()));
    }

    void LookAndFeel::drawComboBox(juce::Graphics& g, int, int, bool, int, int, int, int, juce::ComboBox& box)
    {
        auto bounds = box.getLocalBounds();

        // background
        if (box.getProperties().getWithDefault(Options::INVERT_COLOURS, false))
            g.setColour(findColour(juce::ComboBox::backgroundColourId).getBrightness() > 0.5f ? juce::Colours::black : juce::Colours::white);
        else
            g.setColour(findColour(juce::ComboBox::backgroundColourId));

        g.fillRect(bounds);

        // outline
        if (box.getProperties().getWithDefault(Options::SHOW_OUTLINE, true))
        {
            if (box.getProperties().getWithDefault(Options::INVERT_COLOURS, false))
                g.setColour(findColour(juce::ComboBox::outlineColourId).getBrightness() > 0.5f ? juce::Colours::black : juce::Colours::white);
            else
                g.setColour(findColour(juce::ComboBox::outlineColourId));

            g.drawRect(bounds, defaultThickness<int>);
        }

        if (box.getProperties().getWithDefault(Options::SHOW_ARROW, true))
        {
            auto arrowBounds = bounds.removeFromRight(juce::jmin(bounds.getHeight(), 25)).toFloat().withSizeKeepingCentre(9, 7);

            juce::Path arrow;
            arrow.startNewSubPath(arrowBounds.getTopLeft());
            arrow.lineTo(arrowBounds.getCentreX(), arrowBounds.getBottom());
            arrow.lineTo(arrowBounds.getTopRight());
            arrow.closeSubPath();

            if (box.getProperties().getWithDefault("invertColours", false))
                g.setColour(findColour(juce::ComboBox::arrowColourId).getBrightness() > 0.5f ? juce::Colours::black : juce::Colours::white);
            else
                g.setColour(findColour(juce::ComboBox::arrowColourId));

            g.fillPath(arrow);
        }
    }

    void LookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
    {
        auto bounds = box.getLocalBounds();

        if (box.getProperties().getWithDefault(Options::SHOW_ARROW, true))
            bounds.removeFromRight(juce::jmin(bounds.getHeight(), 25));

        if (box.getJustificationType() != juce::Justification::centred)
            bounds.removeFromLeft(7);

        label.setBounds(bounds);
        label.setFont(getComboBoxFont(box));
        label.getProperties().set(Options::SHOW_OUTLINE, false);
        label.getProperties().set(Options::INVERT_COLOURS,
                                  box.getProperties().getWithDefault(Options::INVERT_COLOURS, false));
    }

    //==================================================================================================================
    void LookAndFeel::drawPluginBackground(juce::Graphics& g, juce::Component& topLevelComponent)
    {
        g.fillAll(findColour(contrast::LookAndFeel::secondaryColourId));

        g.setColour(findColour(contrast::LookAndFeel::primaryColourId));
        g.drawRect(topLevelComponent.getLocalBounds(), defaultThickness<int>);
    }

    //==================================================================================================================
    int LookAndFeel::getHeaderPluginNameWidth(HeaderComponent& header)
    {
        const auto nameFont = font.withHeight(static_cast<float>(header.getHeight()));
        const auto name = juce::String(JucePlugin_Name).toUpperCase();
        return nameFont.getStringWidth(name) + 17;
    }

    void LookAndFeel::drawHeaderComponentBackground(juce::Graphics& g, HeaderComponent& header)
    {
        g.setColour(findColour(primaryColourId));
        g.fillAll();

        auto bounds = header.getLocalBounds();

        {
            const auto nameFont = font.withHeight(static_cast<float>(header.getHeight()));
            const auto name = juce::String(JucePlugin_Name).toUpperCase();
            const auto stringWidth = nameFont.getStringWidth(name) + 10;

            bounds.removeFromLeft(7);
            const auto nameBounds = bounds.removeFromLeft(stringWidth);

            g.setColour(findColour(secondaryColourId));
            g.setFont(nameFont);
            g.drawText(name, nameBounds.translated(0, -1), juce::Justification::centredLeft);
        }
    }

    juce::Image LookAndFeel::createContrastButtonImage(int width, int height)
    {
        juce::Image img(juce::Image::ARGB, width, height, true);
        juce::Graphics g(img);

        auto bounds = img.getBounds().reduced(defaultThickness<int>).toFloat();

        juce::Path path;
        path.addPieSegment(bounds, 0.f, juce::MathConstants<float>::pi, 0.f);

        g.setColour(findColour(secondaryColourId));
        g.fillPath(path);

        path.clear();
        path.addEllipse(bounds);
        g.strokePath(path, juce::PathStrokeType(defaultThickness<float>));

        return img;
    }

    //==================================================================================================================
    void LookAndFeel::setUseWhiteAsPrimaryColour(bool shouldUseWhiteAsPrimaryColour)
    {
        // Value hasn't changed so just return.
        if (shouldUseWhiteAsPrimaryColour == useWhiteAsPrimaryColour)
            return;

        useWhiteAsPrimaryColour = shouldUseWhiteAsPrimaryColour;
        updateColours();

        primaryColourListeners.call([](PrimaryColourListener& listener) {
            listener.primaryColourChanged();
        });
    }

    bool LookAndFeel::isUsingWhiteAsPrimaryColour()
    {
        return useWhiteAsPrimaryColour;
    }

    void LookAndFeel::addPrimaryColourListener(PrimaryColourListener* l)
    {
        primaryColourListeners.add(l);
    }

    void LookAndFeel::removePrimaryColourListener(PrimaryColourListener* l)
    {
        primaryColourListeners.remove(l);
    }

    //==================================================================================================================
    void LookAndFeel::updateColours()
    {
        juce::Colour primary, secondary;

        if (useWhiteAsPrimaryColour)
        {
            primary   = juce::Colours::white;
            secondary = juce::Colours::black;
        }
        else
        {
            primary   = juce::Colours::black;
            secondary = juce::Colours::white;
        }

        setColour(primaryColourId,   primary);
        setColour(secondaryColourId, secondary);

        setColour(juce::Label::backgroundColourId, {});
        setColour(juce::Label::outlineColourId,    primary);
        setColour(juce::Label::textColourId,       primary);

        setColour(juce::Slider::textBoxBackgroundColourId, secondary);
        setColour(juce::Slider::textBoxOutlineColourId,    primary);
        setColour(juce::Slider::textBoxTextColourId,       primary);

        setColour(juce::TextEditor::textColourId,            primary);
        setColour(juce::TextEditor::highlightedTextColourId, secondary);
        setColour(juce::CaretComponent::caretColourId,       primary);
        setColour(juce::PopupMenu::textColourId,             primary);

        setColour(juce::ComboBox::backgroundColourId,     secondary);
        setColour(juce::ComboBox::textColourId,           primary);
        setColour(juce::ComboBox::outlineColourId,        primary);
        setColour(juce::ComboBox::buttonColourId,         secondary);
        setColour(juce::ComboBox::arrowColourId,          primary);
        setColour(juce::ComboBox::focusedOutlineColourId, primary);
    }
}   // namespace contrast
