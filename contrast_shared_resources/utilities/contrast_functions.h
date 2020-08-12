#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    using namespace juce;

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

    /** Returns the nearest whole number to the given value.

        By default this function returns an int but as it is a template
        function, it can be made to return any other type by using the first
        type in the template list.
        For example, contrast::round<float>(4.7) will return 5.f.
    */
    template <typename ReturnType = int, typename InputType>
    inline ReturnType round(InputType value)
    {
        return static_cast<ReturnType>(juce::round(value));
    }

    //==================================================================================================================
    /** Returns a String that can be used to display the given value.
    
        This will round the number to the given number of significant figures
        (where 0 counts as a significant figure) and returns the value as a
        String.
    */
    String pretifyValue(float value, int numSignificantFigures)
    {
        const auto numDigitsBeforePoint = value == 0.f ? 1 : jmax(1, floor(std::log10(std::abs(value)) + 1));

        // If the number of digits before the decimal point (i.e. for 203.1,
        // it would be 3) is greater than the required number of significant
        // figures, round the number to the nearest number of sig figs.
        // (For example 203.1 with 2 sigfigs is 200).
        if (numDigitsBeforePoint > numSignificantFigures)
        {
            const auto difference = numDigitsBeforePoint - numSignificantFigures;
            return String(round(value * std::pow(10.f, -difference)) * std::pow(10.f, difference));
        }

        // Otherwise, if the number of digits before the decimal point is less
        // than the required number of sig figs, we need to round the number to
        // a certain number of decimal places.
        // This includes 0's as significant figures, so 201.03 with 4 sig figs
        // is "201.0" (not "201", or "201.03").
        const auto numDecimalPlaces = numSignificantFigures - numDigitsBeforePoint;

        if (numDecimalPlaces == 0)
            return String(round(value));

        return String(value, numDecimalPlaces);
    }

    //==================================================================================================================
    /** Interpolates between some values using a Lagrange technique. */
    float interpolate(const float* const x, const float* const y, int N, float proportion)
    {
        auto result = 0.f;

        for (int i = 0; i < N; i++)
        {
            auto l = 1.f;

            for (int j = 0; j < N; j++)
            {
                if (j != i)
                    l *= (proportion - x[j]) / (x[i] - x[j]);
            }

            result += l * y[i];
        }

        return result;
    }
}   // namespace contrast
