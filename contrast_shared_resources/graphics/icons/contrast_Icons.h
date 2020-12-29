#pragma once

//======================================================================================================================
namespace contrast
{
    //==================================================================================================================
    namespace Icons
    {
        //==============================================================================================================
        constexpr char contrastIconBlack[] = R"(
            <svg version="1.1" viewBox="0 0 25 25" width="25" height="25" xmlns="http://www.w3.org/2000/svg">
                <rect width="25" height="25" fill="none"/>
                <g stroke="#000" stroke-width="3" stroke-linejoin="round">
                    <circle cx="50%" cy="50%" r="11" fill="none"/>
                    <path d="M 12.5 1.5 A 10.5 10.5 0 0 0 12.5 22.5 Z" fill="#000"/>
                </g>
            </svg>
        )";

        constexpr char contrastIconWhite[] = R"(
            <svg version="1.1" viewBox="0 0 25 25" width="25" height="25" xmlns="http://www.w3.org/2000/svg">
                <rect width="25" height="25" fill="none"/>
                <g stroke="#FFF" stroke-width="3" stroke-linejoin="round">
                    <circle cx="50%" cy="50%" r="11" fill="none"/>
                    <path d="M 12.5 1.5 A 10.5 10.5 0 0 1 12.5 22.5 Z" fill="#FFF"/>
                </g>
            </svg>
        )";

        constexpr char nextIconBlack[] = R"(
            <svg version="1.1" viewBox="0 0 25 25" width="25" height="25" xmlns="http://www.w3.org/2000/svg">
                <rect width="25" height="25" fill="none"/>
                <path d="M 8.5 4.5 L 16.5 12.5 L 8.5 20.5" fill="none" stroke="#000" stroke-width="3"/>
            </svg>
        )";

        constexpr char nextIconWhite[] = R"(
            <svg version="1.1" viewBox="0 0 25 25" width="25" height="25" xmlns="http://www.w3.org/2000/svg">
                <rect width="25" height="25" fill="none"/>
                <path d="M 8.5 4.5 L 16.5 12.5 L 8.5 20.5" fill="none" stroke="#FFF" stroke-width="3"/>
            </svg>
        )";

        constexpr char prevIconBlack[] = R"(
            <svg version="1.1" viewBox="0 0 25 25" width="25" height="25" xmlns="http://www.w3.org/2000/svg">
                <rect width="25" height="25" fill="none"/>
                <path d="M 16.5 4.5 L 8.5 12.5 L 16.5 20.5" fill="none" stroke="#000" stroke-width="3"/>
            </svg>
        )";

        constexpr char prevIconWhite[] = R"(
            <svg version="1.1" viewBox="0 0 25 25" width="25" height="25" xmlns="http://www.w3.org/2000/svg">
                <rect width="25" height="25" fill="none"/>
                <path d="M 16.5 4.5 L 8.5 12.5 L 16.5 20.5" fill="none" stroke="#FFF" stroke-width="3"/>
            </svg>
        )";
    }   // namespace Icons
}   // namespace contrast
