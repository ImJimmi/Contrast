#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    /** Returns the largest whole number below the given value.

        By default this function returns an int but as it is a template
        function, it can be made to return any other type by using the first
        type in the template list.
        For example, contrast::floor<float>(4.7) will return 4.f.
    */
    template <typename ReturnType = int, typename InputType>
    inline ReturnType floor(InputType value)
    {
        return static_cast<ReturnType>(std::floor(value));
    }

    template <typename ReturnType = int, typename InputType>
    inline ReturnType ceil(InputType value)
    {
        return static_cast<ReturnType>(std::ceil(value));
    }

    /** Returns the nearest whole number to the given value.

        By default this function returns an int but as it is a template
        function, it can be made to return any other type by using the first
        type in the template list.
        For example, contrast::round<float>(4.7) will return 5.f.
    */
    template <typename ReturnType = int, typename InputType>
    inline ReturnType round(InputType value)
    {
        return static_cast<ReturnType>(juce::roundToInt(value));
    }

    //==================================================================================================================
    /** Returns a String that can be used to display the given value.
    
        This will round the number to the given number of significant figures
        (where 0 counts as a significant figure) and returns the value as a
        String.
    */
    template <typename ValueType>
    inline juce::String pretifyValue(ValueType value, juce::uint32 numSignificantFigures)
    {
        const auto numDigitsBeforePoint = value == static_cast<ValueType>(0) ? 1U : juce::jmax(1U, floor<juce::uint32>(std::log10(std::abs(value)) + 1U));

        // If the number of digits before the decimal point (i.e. for 203.1,
        // it would be 3) is greater than the required number of significant
        // figures, round the number to the nearest number of sig figs.
        // (For example 203.1 with 2 sigfigs is 200).
        if (numDigitsBeforePoint > numSignificantFigures)
        {
            const auto difference = static_cast<ValueType>(numDigitsBeforePoint - numSignificantFigures);
            return juce::String(round(value * std::pow(static_cast<ValueType>(10), -difference)) * std::pow(static_cast<ValueType>(10), difference));
        }

        // Otherwise, if the number of digits before the decimal point is less
        // than the required number of sig figs, we need to round the number to
        // a certain number of decimal places.
        // This includes 0's as significant figures, so 201.03 with 4 sig figs
        // is "201.0" (not "201", or "201.03").
        const auto numDecimalPlaces = numSignificantFigures - numDigitsBeforePoint;

        if (numDecimalPlaces == 0U)
            return juce::String(round(value));

        return juce::String(value, static_cast<int> (numDecimalPlaces));
    }

    //==================================================================================================================
    /** Interpolates between some values using a Lagrange technique. */
    template <typename ReturnType>
    inline ReturnType interpolate(const ReturnType* const x, const ReturnType* const y,
                                  juce::uint32 N, ReturnType proportion)
    {
        auto result = static_cast<ReturnType>(0);

        for (auto i = 0U; i < N; i++)
        {
            auto l = static_cast<ReturnType>(1);

            for (auto j = 0U; j < N; j++)
            {
                if (j != i)
                    l *= (proportion - x[j]) / (x[i] - x[j]);
            }

            result += l * y[i];
        }

        return result;
    }

    //==================================================================================================================
    /** Initialises the given slider by adding it as a child to the given
        parent, setting some of its properties and finally attaching it to the
        given SliderAttachment object using the parameter ID provided.
    */
    inline void initialiseSlider(juce::Component& parentComponent, juce::AudioProcessorValueTreeState& apvts,
                                 juce::Slider& slider,
                                 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment,
                                 const juce::String& sliderName, const juce::String& correspondingParameterID)
    {
        // Add the slider to the parent and make it visible.
        parentComponent.addAndMakeVisible(slider);

        // Set the slider's name which is used by our custom LookAndFeel class to
        // draw a label above the slider.
        slider.setName(sliderName);

        // Make the slider rotary and have it use a rotary drag.
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);

        // Set the textbox size.
        // The width will be clamped to the slider's width so by setting it to 999
        // we can ensure it'll just be the same width of the slider.
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 999, 25);

        // Initialise the attachment object.
        attachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
            apvts,
            correspondingParameterID,
            slider
        ));
    }

    //==================================================================================================================
    template <typename ReturnType>
    inline ReturnType getRecommendedSliderHeightForWidth(juce::Slider& slider, ReturnType width)
    {
        return width + static_cast<ReturnType>(slider.getTextBoxHeight() * 2);
    }
}   // namespace contrast
