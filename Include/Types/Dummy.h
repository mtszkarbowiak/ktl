
// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

/// <summary>
/// Utility class to be used as a placeholder for a type.
/// Example use include unions, ensuring default constructor, etc.
/// </summary>
class Dummy
{
public:
    Dummy()                                     = default;
    Dummy(const Dummy&)                         = default;
    Dummy(Dummy&&) noexcept                     = default;
    ~Dummy()                                    = default;
    auto operator=(const Dummy&)     -> Dummy&  = default;
    auto operator=(Dummy&&) noexcept -> Dummy&  = default;
};