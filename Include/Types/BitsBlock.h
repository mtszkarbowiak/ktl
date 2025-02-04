// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

namespace BitsStorage
{
    /// <summary> Utility type used to store individual bits. </summary>
    using Block = uint32;

    /// <summary> Number of bytes used to store one block of individual bits. </summary>
    static constexpr int32 BytesPerBlock = sizeof(Block);
    /// <summary> Number of bits stored in one block. </summary>
    static constexpr int32 BitsPerBlock = BytesPerBlock * 8;

    static constexpr Block EmptyBlock = Block{ 0 };
    static constexpr Block FullBlock = ~Block{ 0 };

    /// <summary> Calculates the number of blocks required to store the given number of bits. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto BlocksForBits(const int32 bitCount) -> int32
    {
        return (bitCount + BitsPerBlock - 1) / BitsPerBlock;
    }
}
