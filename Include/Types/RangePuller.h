// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Types/Numbers.h"
#include "Types/SizeHint.h"

/// <summary> Creates a pull iterator of consecutive numbers. </summary>
template<typename T, T Step = 1>
class RangePuller
{
    T _current{};
    T _end{};

public:
    FORCE_INLINE
    RangePuller() = default;

    FORCE_INLINE explicit
    RangePuller(T end)
        : _current{ 0 }
        , _end{ end }
    {
        ASSERT_COLLECTION_SAFE_MOD(Step != 0); // Step must not be zero.
        ASSERT_COLLECTION_SAFE_MOD((Step > 0) == (_current <= _end)); // Step must be positive if end is greater or equal to the current.
    }

    FORCE_INLINE
    RangePuller(T current, T end)
        : _current{ current }
        , _end{ end }
    {
        ASSERT_COLLECTION_SAFE_MOD(Step != 0); // Step must not be zero.
        ASSERT_COLLECTION_SAFE_MOD((Step > 0) == (current <= end)); // Step must be positive if end is greater or equal to the current.
    }


    // Access

    /// <summary> Returns the size hint about the numer of remaining elements. </summary>
    NO_DISCARD FORCE_INLINE
    auto Hint() const -> SizeHint
    {
        const T remaining = (_end - _current + (Step - 1)) / Step;
        return {
            remaining,
            Nullable<T>{ remaining }
        };
    }


    NO_DISCARD FORCE_INLINE
    auto operator*() -> T
    {
        return _current;
    }

    NO_DISCARD FORCE_INLINE
    auto operator->() -> T*
    {
        return &_current;
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() const -> const T
    {
        return _current;
    }

    NO_DISCARD FORCE_INLINE
    auto operator->() const -> const T*
    {
        return &_current;
    }


    // Iteration

    NO_DISCARD FORCE_INLINE explicit
    operator bool() const
    {
        return _current < _end;
    }

    MAY_DISCARD FORCE_INLINE 
    auto operator++() -> RangePuller&
    {
        ASSERT_COLLECTION_SAFE_MOD(_current <= _end); // Cannot increment past the end.
        _current += Step;
        return *this;
    }

    MAY_DISCARD FORCE_INLINE
    auto operator++(int) -> RangePuller
    {
        RangePuller copy{ *this };
        _current += Step;
        return copy;
    }


    // Identity

    NO_DISCARD FORCE_INLINE
    auto operator==(const RangePuller& other) const -> bool 
    {
        return _current == other._current && _end == other._end;
    }

    NO_DISCARD FORCE_INLINE
    auto operator!=(const RangePuller& other) const -> bool
    {
        return !(*this == other);
    }

    NO_DISCARD FORCE_INLINE
    auto operator<(const RangePuller& other) const -> bool
    {
        return _current < other._current && _end < other._end;
    }


    // Constraints

    static_assert(!TIsRefV<T>, "RawPuller cannot be used with references.");
    static_assert(!TIsConstV<T>, "RawPuller cannot be used with const types.");
};