// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include <Language/TypeTraits.h>
#include <Debugging/Assertions.h>

/// <summary>
/// Iterator for enum classes. Does not support empty enum classes.
/// </summary>
/// <typeparam name="T"> Enum class type. </typeparam>
/// <typeparam name="Last"> Last value of the enum class. </typeparam>
/// <typeparam name="First"> First value of the enum class. </typeparam>
/// <typeparam name="U"> Underlying type of the enum class. </typeparam>
template<
    typename T,
    T Last  = T{},
    T First = T{},
    typename U = typename TUnderlyingType<T>::Type
>
class EnumPuller 
{
PRIVATE:
    static constexpr U UBegin = static_cast<U>(First);
    static constexpr U UEnd   = static_cast<U>(Last) + 1;
    static constexpr U UCount = UEnd - UBegin;

    static_assert(UBegin < UEnd, "Begin must be less than End.");

    U _current = UBegin;

public:
    NO_DISCARD FORCE_INLINE explicit
    operator bool() const
    {
        return _current < UEnd;
    }

    MAY_DISCARD FORCE_INLINE
    auto operator++() -> EnumPuller&
    {
        ++_current;
        return *this;
    }

    MAY_DISCARD FORCE_INLINE
    auto operator++(int) -> EnumPuller
    {
        auto copy = *this;
        ++*this;
        return copy;
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() const -> T
    {
        ASSERT(_current < UEnd); // Out of range.
        return static_cast<T>(_current);
    }
    

    NO_DISCARD FORCE_INLINE
    auto Hint() const -> class SizeHint;
};
