// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/CollectionsUtils.h"

/// <summary> Utility type used to store individual bits. </summary>
using BitsBlock = uint32;

/// <summary> Utility class that allows accessing the bit at the specified index using the assignment operator. </summary>
class MutBitRef final
{
    BitsBlock* _block;
    int32      _index;

public:
    FORCE_INLINE explicit
    MutBitRef(BitsBlock* block, const int32 index)
        : _block{ block }
        , _index{ index }
    {
    }

    MAY_DISCARD FORCE_INLINE
    auto operator=(const bool value) -> MutBitRef&
    {
        const BitsBlock mask = BitsBlock{ 1 } << _index;

        if (value)
            *_block |= mask;
        else
            *_block &= ~mask;

        return *this;
    }

    NO_DISCARD FORCE_INLINE
    operator bool() const
    {
        const BitsBlock mask = BitsBlock{ 1 } << _index;
        return (*_block & mask) != 0;
    }
};

/// <summary> Utility class that allows accessing the bit at the specified index using the assignment operator. </summary>
class ConstBitRef final
{
    const BitsBlock* _block;
    int32            _index;

public:
    FORCE_INLINE explicit
    ConstBitRef(const BitsBlock* block, const int32 index)
        : _block{ block }
        , _index{ index }
    {
    }

    auto operator=(bool value) -> ConstBitRef& = delete;

    NO_DISCARD FORCE_INLINE
    operator bool() const
    {
        const BitsBlock mask = BitsBlock{ 1 } << _index;
        return (*_block & mask) != 0;
    }
};
