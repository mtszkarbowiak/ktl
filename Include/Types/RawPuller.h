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

/// <summary> Creates a universal pull iterator for elements stored in a contiguous memory block. </summary>
template<typename T>
class RawPuller
{
    T* _begin{};
    T* _end{};

public:
    RawPuller() = default;

    FORCE_INLINE
    RawPuller(T* begin, T* end)
        : _begin{ begin }
        , _end{ end }
    {
        ASSERT_COLLECTION_SAFE_MOD(begin <= end); // Span begin must be before the end.
    }


    // Access

    /// <summary> Returns the size hint about the numer of remaining elements. </summary>
    NO_DISCARD FORCE_INLINE
    auto Hint() const -> SizeHint
    {
        const int32 remaining = static_cast<int32>(_end - _begin);
        return {
            remaining,
            Nullable<Index>{ remaining }
        };
    }

    
    NO_DISCARD FORCE_INLINE
    auto operator*() -> T&
    {
        return *_begin;
    }

    NO_DISCARD FORCE_INLINE
    auto operator->() -> T*
    {
        return _begin;
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() const -> const TRemoveConstT<T>&
    {
        return *_begin;
    }

    NO_DISCARD FORCE_INLINE
    auto operator->() const -> const TRemoveConstT<T>*
    {
        return _begin;
    }


    // Iteration

    /// <summary> Check if the enumerator points to a valid element. </summary>
    NO_DISCARD FORCE_INLINE explicit
    operator bool() const
    {
        return _begin < _end;
    }

    /// <summary> Moves the enumerator to the next element. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator++() -> RawPuller&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_begin < _end); // Iterator must not be at the end to be incremented.
        ++_begin;
        return *this;
    }

    /// <summary> Moves the enumerator to the next element. </summary>
    /// <remarks> Prefixed increment operator is faster. </remarks>
    MAY_DISCARD FORCE_INLINE
    auto operator++(int) -> RawPuller
    {
        RawPuller copy = *this;
        ++(*this);
        return copy;
    }


    // Identity

    NO_DISCARD FORCE_INLINE
    auto operator==(const RawPuller& other) const -> bool
    {
        return _begin == other._begin && _end == other._end;
    }

    NO_DISCARD FORCE_INLINE
    auto operator!=(const RawPuller& other) const -> bool
    {
        return _begin != other._begin || _end != other._end;
    }

    NO_DISCARD FORCE_INLINE
    auto operator<(const RawPuller& other) const -> bool
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_end == other._end); // Enumerators must be of the same span to be compared.
        return _begin < other._begin;
    }


    // Constraints

    static_assert(!TIsRefV<T>, "RawPuller cannot be used with references.");
};
