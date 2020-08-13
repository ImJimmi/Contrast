#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    class PitchShifter
    {
    public:
        //==============================================================================================================
        PitchShifter(const int maximumDelay, double sampleRate, juce::uint32 blockSize)
            :   maxDelay(static_cast<float>(maximumDelay))
        {
            delayLength = maximumDelay - 24;
            halfLength = delayLength / 2;
            delay[0] = 12.f;
            delay[1] = maxDelay / 2.f;

            for (int i = 0; i < 2; i++)
            {
                delayLine[i].reset(new juce::dsp::DelayLine<float>(maximumDelay));
                delayLine[i]->prepare({ sampleRate, blockSize, 1 });
                delayLine[i]->setDelay(delay[0]);
            }
        }

        //==============================================================================================================
        float processSample(float input)
        {
            // Update the delay lengths and clamp them between the valid range.
            delay[0] += rate;
            delay[1] = delay[0] + halfLength;

            for (int i = 0; i < 2; i++)
            {
                while (delay[i] > maxDelay - 12.f)
                    delay[i] -= delayLength;
                while (delay[i] < 12.f)
                    delay[i] += delayLength;

                // Set the new delay line length.
                delayLine[i]->setDelay(delay[i]);
            }

            // Calculate a triangular envelope.
            envelope[1] = std::abs((delay[0] - halfLength + 12.f) * (1.f / (halfLength + 12.f)));
            envelope[0] = 1.f - envelope[1];

            // Delay the input.
            delayLine[0]->pushSample(0, input);
            delayLine[1]->pushSample(0, input);

            // Get the delayed signal and apply the envelope.
            auto output = envelope[0] * delayLine[0]->popSample(0);
            output += envelope[1] * delayLine[1]->popSample(0);

            // Apply the mix.
            output *= wetMix;
            output += dryMix * input;

            return output;
        }

        //==============================================================================================================
        /** Sets the frequency shift of the pitch shifter.
            This is essentially the number by which the frequency content of
            the sound will be multiplied, so a shift of 2 will result in the
            sound being an octave higher.
        */
        void setShift(float shift)
        {
            if (shift != 0.f)
                rate = 1.f - shift;
            else
            {
                rate = 0.f;
                delay[0] = static_cast<float>(halfLength) + 12.f;
            }
        }

        /** Sets the amount of shift in frequency to use.
            This overloaded method takes the number of semitones and cents to
            shift the frequency by which is then converted to the shift value.
            For example, +7 semitones gives a shift of 1.5.
        */
        void setShift(int semitones, float cents)
        {
            setShift(std::pow(twelthRoot2, semitones + cents / 100.f));
        }

        /** Returns the current length of the delay being applied by this
            effect.
        */
        int getDelayLength()
        {
            return delayLength;
        }

        /** Sets the dry/wet mix for the effect.
            Value must be 0 - 1 where 0 is dry and 1 is wet.
        */
        void setMix(float newMix)
        {
            jassert(newMix >= 0.f && newMix <= 1.f);

            // By using the sqrt of the mix value, the overall loudness of the
            // output signal after the mix has been applied will be a little
            // louder when the mix is > 0 and < 1.
            wetMix = std::sqrt(newMix);
            dryMix = std::sqrt(1.f - newMix);
        }

    private:
        //==============================================================================================================
        std::unique_ptr<juce::dsp::DelayLine<float>> delayLine[2];

        float delay[2] = { 0.f, 0.f };
        const float maxDelay = 0.f;
        float envelope[2] = { 0.f, 0.f };
        float rate = 1.f;
        int delayLength = 0;
        int halfLength = 0;
        float wetMix = 1.f;
        float dryMix = 0.f;

        static const float twelthRoot2;
    };

    //==================================================================================================================
    inline const float PitchShifter::twelthRoot2 = std::pow(2.f, 1.f / 12.f);
}   // namespace contrast
