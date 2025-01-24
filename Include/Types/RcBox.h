// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Types/Box.h"
#include "Types/Rc.h"


template<
    typename  T,
    typename  A = HeapAlloc
>
class RcBox
{
    Box<T, A> _box;
    Rc        _rc;

public:
    NO_DISCARD FORCE_INLINE
    auto Rc() const -> const Rc&
    {
        return _rc;
    }

    NO_DISCARD FORCE_INLINE
    void Reset()
    {
        ASSERT_COLLECTION_SAFE_MOD(!_rc.HasAnyRefs());
        _box.Reset();
    }

    NO_DISCARD FORCE_INLINE
    auto operator->() -> T*
    {
        return _box.Get();
    }

    NO_DISCARD FORCE_INLINE
    auto operator->() const -> const T*
    {
        return _box.Get();
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() -> T&
    {
        return *_box.Get();
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() const -> const T&
    {
        return *_box.Get();
    }


    NO_DISCARD FORCE_INLINE
    auto TryRead() -> RcRead<T>
    {
        if (_box.IsEmpty() || !_rc.CanRead()) 
        {
            return RcRead<T>{};
        }
        return RcRead<T>{&_rc, _box.Get()};
    }

    NO_DISCARD FORCE_INLINE
    auto TryReadLimited(const Rc::IntRc limit) -> RcRead<T>
    {
        if (_box.IsEmpty() || !_rc.CanRead() || (_rc.GetReads() >= limit))
        {
            return RcRead<T>{};
        }
        return RcRead<T>{&_rc, _box.Get()};
    }

    NO_DISCARD FORCE_INLINE
    auto TryWrite() -> RcWrite<T>
    {
        if (_box.IsEmpty() || !_rc.CanWrite()) 
        {
            return RcWrite<T>{};
        }
        return RcWrite<T>{&_rc, _box.Get()};
    }
};
