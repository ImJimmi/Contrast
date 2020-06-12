#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    /** A super simple envelope follower class that uses an expenential function
        for the attack and release - this means the signal doesn't actually
        reach its target level in the specified time (and in theory, never
        does), but it gets close enough to not be noticable.
    */
    class EnvelopeFollower
    {
    public:
        //==============================================================================================================
        /** The sample rate is required when calculating the attack and release
            coeficients so it must be given here to ensure it's available later.
        */
        EnvelopeFollower(float sampleRate)
            :   samplesPerMS(sampleRate / 1000.f)
        {
        }

        //==============================================================================================================
        /** Calculates and returns the new envelope value from the given input,
            applying attack and release.
        */
        float processSample(float input)
        {
            input = std::abs(input);

            // Apply the attack if the input is above the current envelope
            if (input > currentEnvelope)
                currentEnvelope = attack * (currentEnvelope - input) + input;
            // Apply the release if the input is below the current envelope.
            else
                currentEnvelope = release * (currentEnvelope - input) + input;

            // Returns the new envelope value.
            return currentEnvelope;
        }

        /** Returns the most recently calculated envelope value. */
        float getCurrentEnvelope()
        {
            return currentEnvelope;
        }

        //==============================================================================================================
        /** Changes the time it takes for the envelope to respond to the input
            signal being louder than the current envelope.

            The given newAttackTimeInMS value must be in milliseconds.
        */
        void setAttackTime(float newAttackTimeInMS)
        {
            attack = std::exp(std::log(0.01f) / (newAttackTimeInMS * samplesPerMS));
        }

        /** Changes the time it takes for the envelope to response to the input
            signal being quieter than the current envelope.

            The given newReleaseTimeInMS value must be in milliseconds.
        */
        void setReleaseTime(float newReleaseTimeInMS)
        {
            release = std::exp(std::log(0.01f) / (newReleaseTimeInMS * samplesPerMS));
        }

    private:
        //==============================================================================================================
        // The number of samples per millisecond (as opposed to per second, as
        // a 'normal' sample rate would be).
        const float samplesPerMS;

        // The most recently calculated envelope value.
        float currentEnvelope = 0.f;

        // The attack coefficient. This is not the actual attack time in
        // milliseconds but rather a constant we can use to easy the envelope.
        float attack = 0.f;

        // The release coefficient.
        float release = 0.f;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeFollower)
    };
}   // namespace contrast
