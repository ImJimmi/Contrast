#pragma once

//======================================================================================================================
namespace Gate
{
    //==================================================================================================================
    // The IDs of the parameters for this plugin.
    namespace ParameterIDs
    {
        constexpr char THRESHOLD[] = "threshold";
        constexpr char ATTACK[]    = "attack";
        constexpr char RELEASE[]   = "release";
    }   // namespace ParameterIDs

    //==================================================================================================================
    // Constants

    template <typename T>
    constexpr T releaseMin = static_cast<T>(20);

    template <typename T>
    constexpr T releaseMax = static_cast<T>(2000);
}   // namespace Gate
