#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    class Compressor
    {
    public:
        //==============================================================================================================
        Compressor(float sampleRate)
            :   follower(sampleRate)
        {
            follower.setAttackTime(20.f);
            follower.setReleaseTime(1000.f);
        }

        //==============================================================================================================
        float calculateGain(float envelopeDB)
        {
            // Calculate the initial slope value, assuming there's no interpolation needed.
            auto slope = 1.f - 1.f / ratio;

            // Interpolate the output if it is within the knee limits
            if (knee > 0.f && envelopeDB > (threshold - knee / 2.f) && envelopeDB < (threshold + knee / 2.f))
            {
                float x[] = { threshold - knee / 2.f, juce::jmin(0.f, threshold + knee / 2.f) };
                float y[] = { 0.f, slope };

                slope = interpolate(x, y, 2, envelopeDB);
            }

            // Calculate the gain in decibels.
            auto decibels = juce::jmin(0.f, slope * (threshold - envelopeDB));

            // Convert the decibels to gain and return the result.
            return juce::Decibels::decibelsToGain(decibels);
        }
        
        float processSample(float input)
        {
            auto envelopeDB = juce::Decibels::gainToDecibels(follower.processSample(input));

            // If the new envelope value is below the lower limit of the knee,
            // we can just return the input because we don't need to apply gain.
            if (envelopeDB <= threshold - knee / 2.f)
                return input;

            // If not, we need to calculate the gain to apply.
            auto gain = calculateGain(envelopeDB);

            // Apply compressor gain and makeup gain to form the output.
            return gain * input * makeupGain;
        }

        //==============================================================================================================
        void setThreshold(float newThreshold)
        {
            jassert(newThreshold <= 0.f);
            threshold = newThreshold;
        }

        void setRatio(float newRatio)
        {
            jassert(newRatio >= 1.f);
            ratio = newRatio;
        }

        void setKnee(float newKnee)
        {
            jassert(newKnee >= 0.f);
            knee = newKnee;
        }

        void setMakeupGain(float newMakeupDB)
        {
            makeupGain = juce::Decibels::decibelsToGain(newMakeupDB);
        }

        void setAttack(float newAttackTimeMS)
        {
            follower.setAttackTime(newAttackTimeMS);
        }

        void setRelease(float newReleaseTimeMS)
        {
            follower.setReleaseTime(newReleaseTimeMS);
        }

    private:
        //==============================================================================================================
        EnvelopeFollower follower;

        float threshold = 0.f;
        float ratio = 1.f;
        float knee = 0.f;
        float makeupGain = 1.f;
    };
}   // namespace contrast
