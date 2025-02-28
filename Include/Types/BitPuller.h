// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/CollectionsUtils.h"
#include "Types/BitRef.h"

/// <summary>
/// Iterator for the array which provides end condition and allows to iterate over the elements in a range-based for loop.
/// </summary>
/// <remarks>
/// Warning: Dereferencing the iterator returns a mutable bit reference, not a value.
/// </remarks>
class BitMutPuller
{
    BitsStorage::Block* _blocks;
    int32      _currentIndex;
    int32      _endIndex;


public:
    FORCE_INLINE explicit
    BitMutPuller(BitsStorage::Block* blocks, const int32 beginIndex, const int32 endIndex)
        : _blocks{ blocks }
        , _currentIndex{ beginIndex }
        , _endIndex{ endIndex }
    {
    }


    // Identity

    NO_DISCARD FORCE_INLINE
    auto operator==(const BitMutPuller& other) const -> bool
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_blocks == other._blocks);
        return _currentIndex == other._currentIndex;
    }

    NO_DISCARD FORCE_INLINE
    auto operator!=(const BitMutPuller& other) const -> bool
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_blocks == other._blocks);
        return _currentIndex != other._currentIndex;
    }

    NO_DISCARD FORCE_INLINE
    auto operator<(const BitMutPuller& other) const -> bool
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_blocks == other._blocks);
        return _currentIndex < other._currentIndex;
    }


    // Access

    /// <summary> Returns the size hint about the numer of remaining elements. </summary>
    NO_DISCARD FORCE_INLINE
    auto Hint() const -> SizeHint
    {
        const int32 remaining = _endIndex - _currentIndex;
        return SizeHint::Exactly(remaining);
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() -> MutBitRef
    {
        using namespace BitsStorage;

        const int32 blockIndex = _currentIndex / BitsPerBlock;
        const int32 bitIndex   = _currentIndex % BitsPerBlock;

        return MutBitRef{
            _blocks + blockIndex,
            bitIndex
        };
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() const -> ConstBitRef
    {
        using namespace BitsStorage;

        const int32 blockIndex = _currentIndex / BitsPerBlock;
        const int32 bitIndex   = _currentIndex % BitsPerBlock;

        return ConstBitRef{
            _blocks + blockIndex,
            bitIndex
        };
    }


    // End Condition

    /// <summary> Check if the puller reached the end of the array. </summary>
    NO_DISCARD FORCE_INLINE explicit
    operator bool() const 
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_blocks != nullptr);
        return _currentIndex < _endIndex;
    }

    /// <summary> Returns the index of the current element. </summary>
    NO_DISCARD FORCE_INLINE
    auto Index() const -> int32
    {
        return _currentIndex;
    }


    // Movement

    /// <summary> Moves the puller to the next element. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator++() -> BitMutPuller&
    {
        ++_currentIndex;
        return *this;
    }

    /// <summary> Moves the puller to the next element. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator++(int) -> BitMutPuller
    {
        BitMutPuller copy{ *this };
        ++_currentIndex;
        return copy;
    }
};

/// <summary>
/// Iterator for the array which provides end condition and allows to iterate over the elements in a range-based for loop.
/// </summary>
/// <remarks>
/// Warning: Dereferencing the iterator returns a const bit reference, not a value.
/// </remarks>
class BitConstPuller
{
    const BitsStorage::Block* _blocks;
    int32            _currentIndex;
    int32            _endIndex;


public:
    FORCE_INLINE explicit
    BitConstPuller(
        const BitsStorage::Block* array,
        const int32 beginIndex, 
        const int32 endIndex)
        : _blocks{ array }
        , _currentIndex{ beginIndex }
        , _endIndex{ endIndex }
    {
    }


    // Identity

    NO_DISCARD FORCE_INLINE
    auto operator==(const BitConstPuller& other) const -> bool
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_blocks == other._blocks);
        return _currentIndex == other._currentIndex;
    }

    NO_DISCARD FORCE_INLINE
    auto operator!=(const BitConstPuller& other) const -> bool
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_blocks == other._blocks);
        return _currentIndex != other._currentIndex;
    }

    NO_DISCARD FORCE_INLINE
    auto operator<(const BitConstPuller& other) const -> bool
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_blocks == other._blocks);
        return _currentIndex < other._currentIndex;
    }


    // Access

    /// <summary> Returns the size hint about the numer of remaining elements. </summary>
    NO_DISCARD FORCE_INLINE
    auto Hint() const -> SizeHint
    {
        const int32 remaining = _endIndex - _currentIndex;
        return SizeHint::Exactly(remaining);
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() const -> ConstBitRef
    {
        using namespace BitsStorage;
        const int32 blockIndex = _currentIndex / BitsPerBlock;
        const int32 bitIndex   = _currentIndex % BitsPerBlock;
        return ConstBitRef{ _blocks + blockIndex, bitIndex };
    }


    // End Condition and Movement

    NO_DISCARD FORCE_INLINE explicit
    operator bool() const 
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_blocks != nullptr);
        return _currentIndex < _endIndex;
    }

    MAY_DISCARD FORCE_INLINE
    auto operator++() -> BitConstPuller&
    {
        ++_currentIndex;
        return *this;
    }

    MAY_DISCARD FORCE_INLINE
    auto operator++(int) -> BitConstPuller
    {
        BitConstPuller copy{ *this };
        ++_currentIndex;
        return copy;
    }
};
