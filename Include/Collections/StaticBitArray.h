// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/CollectionsUtils.h"
#include "Types/BitPuller.h"

/// <summary>
/// A container for statically sized arrays of bits, stored in a single contiguous block of memory.
/// </summary>
template<uintptr N>
class StaticBitArray
{
public:
    using Block       = uint64;
    using MutPuller   = BitMutPuller;
    using ConstPuller = BitConstPuller;


protected:
    /// <summary> Calculates the number of blocks required to store the given number of bits. </summary>
    static FORCE_INLINE constexpr
    auto BlocksForBits(const int32 bitCount) -> int32
    {
        return (bitCount + BitsPerBlock - 1) / BitsPerBlock;
    }

    static constexpr int32 BitsPerBlock = sizeof(Block) * 8;


PRIVATE:
    static constexpr int32 BitCount = N;
    static constexpr int32 BlockCount = BlocksForBits(N);

    Block _data[BlockCount];


    // Lifecycle

public:
    /// <summary> Initializes the array with zeros. </summary>
    FORCE_INLINE constexpr
    StaticBitArray()
        : _data{}
    {
    }

    /// <summary> Initializes the array by copying another array. </summary>
    FORCE_INLINE constexpr
    StaticBitArray(const StaticBitArray& other) = default;

    /// <summary> Initializes the array by moving another array. </summary>
    FORCE_INLINE constexpr
    StaticBitArray(StaticBitArray&& other) NOEXCEPT_S = default;

    /// <summary> Destroys the array. </summary>
    FORCE_INLINE
    ~StaticBitArray() = default;

    /// <summary> Assigns another array to this one by copying. </summary>
    MAY_DISCARD FORCE_INLINE constexpr
    auto operator=(const StaticBitArray& other) -> StaticBitArray& = default;

    /// <summary> Assigns another array to this one by moving. </summary>
    MAY_DISCARD FORCE_INLINE constexpr
    auto operator=(StaticBitArray&& other) NOEXCEPT_S -> StaticBitArray& = default;
    

    // Element Access

    /// <summary> Accesses the bit at the specified index. </summary>
    /// <remarks>
    /// This method uses a proxy object to allow the assignment operator to be used.
    /// To read bit without overhead use <c>GetBit</c> method.
    /// </remarks>
    NO_DISCARD FORCE_INLINE
    auto operator[](const int32 index) const -> ConstBitRef
    {
        const int32 blockIndex = index / BitsPerBlock;
        const int32 bitIndex   = index % BitsPerBlock;
        return ConstBitRef{ _data + blockIndex, bitIndex };
    }

    /// <summary> Accesses the bit at the specified index. </summary>
    /// <remarks>
    /// This method uses a proxy object to allow the assignment operator to be used.
    /// To modify bit without overhead use <c>SetBit</c> method.
    /// </remarks>
    NO_DISCARD FORCE_INLINE
    auto operator[](const int32 index) -> MutBitRef
    {
        const int32 blockIndex = index / BitsPerBlock;
        const int32 bitIndex   = index % BitsPerBlock;
        return MutBitRef{ _data + blockIndex, bitIndex };
    }


    // Element Manipulation

    /// <summary> Returns the bit value. </summary>
    /// <param name="index"> Index of the bit to access. Must be in the range [0, Count). </param>
    /// <returns> Value of the bit at the specified index. </returns>
    NO_DISCARD FORCE_INLINE
    auto GetBit(const int32 index) const -> bool
    {
        ASSERT_COLLECTION_SAFE_ACCESS(index >= 0 && index < BitCount);

        const int32 blockIndex = index / BitsPerBlock;
        const int32 bitIndex   = index % BitsPerBlock;

        const Block* srcBlock = _data + blockIndex;
        const Block  mask     = Block{ 1 } << bitIndex;
        const bool   result   = (*srcBlock & mask) != 0;

        return result;
    }

    /// <summary> Sets the bit value. </summary>
    /// <param name="index"> Index of the bit to set. Must be in the range [0, Count). </param>
    /// <param name="value"> Value to set. </param>
    FORCE_INLINE
    void SetBit(const int32 index, const bool value)
    {
        ASSERT_COLLECTION_SAFE_MOD(index >= 0 && index < BitCount);

        const int32 blockIndex = index / BitsPerBlock;
        const int32 bitIndex   = index % BitsPerBlock;

        Block* dstBlock  = _data + blockIndex;
        const Block mask = Block{ 1 } << bitIndex;

        if (value)
            *dstBlock |= mask;
        else
            *dstBlock &= ~mask;
    }

    /// <summary> Sets all bits to the specified value. </summary>
    FORCE_INLINE
    void SetAll(const bool value)
    {
        if (BitCount == 0)
            return;

        const Block fillValue   = value ? ~Block{} : Block{};
        const int32 blocksCount = BlocksForBits(BitCount);

        Block* blocks = DATA_OF(Block, _allocData);
        for (int32 i = 0; i < blocksCount; ++i)
            blocks[i] = fillValue;
    }

    
    // Iterators

    NO_DISCARD FORCE_INLINE
    auto Values() -> BitMutPuller
    {
        return BitMutPuller{ _data, 0, BitCount };
    }

    NO_DISCARD FORCE_INLINE
    auto Values() const -> BitConstPuller
    {
        return BitConstPuller{ _data, 0, itCount };
    }
};

//TODO(mtszkarbowiak): Introduce specialized `BitsBlock` type shared between `BitArray` and `StaticBitArray`.
