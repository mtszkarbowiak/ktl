// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/TypeTraits.h"

#if CONCEPTS_ENABLED

template<typename A>
concept AllocatorData2 = requires(A alloc, const A& constAlloc)
{
    { alloc.Allocate(int32{}) }   -> SameAs<int32>;
    { alloc.Reallocate(int32{}) } -> SameAs<int32>;
    { alloc.Free() };

    { constAlloc.MovesItems() } -> SameAs<bool>;

    { alloc.Get() }      -> SameAs<byte*>;
    { constAlloc.Get() } -> SameAs<const byte*>;

    //TODO(mtszkarbowiak): Lifecycle methods
};

template<typename A>
concept Allocator = requires(A alloc)
{
    typename A::Data;
    requires AllocatorData2<typename A::Data>;

    { A::IsNullable };
    { A::MaxCapacity };
    { A::MinCapacity };

    requires TIsSameV<TRemoveCVRefT<decltype(A::IsNullable)>, bool>;
    requires TIsSameV<TRemoveCVRefT<decltype(A::MinCapacity)>, int32>;
    requires TIsSameV<TRemoveCVRefT<decltype(A::MaxCapacity)>, int32>;
};

#endif
