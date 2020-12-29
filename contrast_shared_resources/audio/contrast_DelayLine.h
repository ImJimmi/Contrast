#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    /** Manages the logic required to implement a variable-length delay line.
        A maximum length must be given as a template argument so as to avoid
        reallocations when the length of the delay line is changed.

        Reading and writing from this delay line should only be done from a
        single thread (most likely the audio thread) but the length can be set
        from a different thread if needed.

        This class is lock-free, making it safe for use on a real-time thread.
    */
    template <typename ValueType>
    class DelayLine
    {
    public:
        //==============================================================================================================
        DelayLine(std::size_t delayLineCapacity)
            :   capacity(delayLineCapacity),
                delayLine(delayLineCapacity, static_cast<ValueType>(0))
        {
            reset();
        }

        DelayLine(const DelayLine& other)
            :   delayLine (other.delayLine),
                writeIndex(other.writeIndex),
                length    (other.length.load())
        {
        }

        //==============================================================================================================
        /** Sets the current length of the delay line. */
        void setLength(int newLength)
        {
            jassert(newLength <= capacity && newLength >= 0);
            length = newLength;
        }

        /** Writes a new value to the delay line.
        */
        void write(ValueType newValue)
        {
            // First, increment the write index.
            writeIndex++;

            // Then wrap it around to 0 if it's behond the capacity of the delay
            // line.
            if (writeIndex == capacity)
                writeIndex = 0;

            // Write the new value to the newly calculated index in the delay
            // line.
            delayLine[writeIndex] = newValue;
        }

        /** Returns a value from the delay line that's N samples behind the most
            recently added one (where N is the current delay line length).
        */
        ValueType read()
        {
            // Find the index in the delay line to read from.
            // The index should be N less than the write index, wrapped around
            // if that makes it negative.
            auto index = writeIndex - length;

            if (index < 0)
                index += capacity;

            // Return the value from the delay line with the calculated index.
            return delayLine[index];
        }

        /** Resets the delay line to zeros. */
        void reset()
        {
            for (auto& value : delayLine)
                value = static_cast<ValueType>(0);
        }

    private:
        //==============================================================================================================
        // The maximum size of the delayLine.
        const std::size_t capacity;

        // The data stored in the delayline.
        std::vector<ValueType> delayLine;

        // The current write index.
        int writeIndex{ 0 };

        // The current length of the delay line.
        std::atomic<int> length;
        static_assert(std::atomic<int>::is_always_lock_free);
    };
}   // namespace contrast
