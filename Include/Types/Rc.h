// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Debugging/Assertions.h"
#include "Types/Numbers.h"

/// <summary>
/// Non-thread safe reference counter.
/// </summary>
class Rc
{
public:
    using IntRc = int16;

PRIVATE:
    // Positive values are reads.
    // Negative values are writes.
    IntRc _counter{};


    // Element Access

public:
    NO_DISCARD FORCE_INLINE
    auto HasAnyRefs() const -> bool
    {
        return _counter != 0;
    }

    NO_DISCARD FORCE_INLINE
    auto GetReads() const -> IntRc
    {
        if (_counter >= 0)
        {
            return _counter;
        }
        return 0;
    }

    NO_DISCARD FORCE_INLINE
    auto GetWrites() const -> IntRc
    {
        if (_counter <= 0)
        {
            return static_cast<IntRc>(-_counter);
        }
        return 0;
    }


    NO_DISCARD FORCE_INLINE
    auto CanRead() const -> bool
    {
        return _counter >= 0;
    }

    NO_DISCARD FORCE_INLINE
    auto CanWrite() const -> bool
    {
        return _counter <= 0;
    }


    FORCE_INLINE
    void AddRead()
    {
        ASSERT_COLLECTION_SAFE_MOD(_counter >= 0);
        ++_counter;
    }

    FORCE_INLINE
    void RemoveRead()
    {
        ASSERT_COLLECTION_SAFE_MOD(_counter > 0);
        --_counter;
    }

    FORCE_INLINE
    void AddWrite()
    {
        ASSERT_COLLECTION_SAFE_MOD(_counter == 0); // Only one write at a time.
        --_counter;
    }

    FORCE_INLINE
    void RemoveWrite()
    {
        ASSERT_COLLECTION_SAFE_MOD(_counter < 0);
        ++_counter;
    }


    // Lifecycle

    FORCE_INLINE
    Rc() = default;

    FORCE_INLINE
    Rc(Rc&& other) noexcept
    {
        // Counter is already initialized.
        ASSERT_COLLECTION_SAFE_MOD(other._counter == 0);
    }

    MAYBE_UNUSED FORCE_INLINE
    auto operator=(Rc&& other) noexcept -> Rc&
    {
        ASSERT_COLLECTION_SAFE_MOD((other._counter == 0) && (_counter == 0));
        return *this;
    }

    FORCE_INLINE
    ~Rc()
    {
        ASSERT_COLLECTION_SAFE_MOD(_counter == 0); // All references must be released.
    }


    // Disallow copying

    Rc(const Rc& copied) = delete;

    auto operator=(const Rc& copied) -> Rc& = delete;
};

template<typename T>
class RcRead
{
PRIVATE:
    const T* _ptr{};
    Rc*      _rc{};


    // Element Access

public:
    NO_DISCARD FORCE_INLINE
    auto HasValue() const -> bool
    {
        return _ptr != nullptr;
    }

    NO_DISCARD FORCE_INLINE
    auto IsUnique() const -> bool
    {
        return static_cast<bool>(_rc) && _rc->GetReads() == 1;
    }

    NO_DISCARD FORCE_INLINE
    auto Get() const -> const T*
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_ptr != nullptr);
        return _ptr;
    }

    NO_DISCARD FORCE_INLINE
    auto operator->() const -> const T*
    {
        return Get();
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() const -> const T&
    {
        return *Get();
    }

    FORCE_INLINE
    void Clear()
    {
        if (_rc)
            _rc->RemoveRead();

        _ptr = nullptr;
        _rc = nullptr;
    }


    // Lifecycle

    FORCE_INLINE
    RcRead() = default;

    FORCE_INLINE
    RcRead(RcRead&& other) noexcept
    {
        ::Swap(_ptr, other._ptr);
        ::Swap(_rc, other._rc);
    }

    FORCE_INLINE
    RcRead(const RcRead& copied)
        : _ptr{ copied._ptr }
        , _rc{ copied._rc }
    {
        if (_rc)
            _rc->AddRead();
    }

    FORCE_INLINE
    RcRead(Rc& rc, const T& ptr)
        : _ptr{ &ptr }
        , _rc{ &rc }
    {
        _rc->AddRead();
    }

    FORCE_INLINE
    ~RcRead()
    {
        Clear();
    }


    MAYBE_UNUSED FORCE_INLINE
    auto operator=(RcRead&& other) noexcept -> RcRead&
    {
        Clear();

        ::Swap(_ptr, other._ptr);
        ::Swap(_rc, other._rc);

        return *this;
    }

    MAYBE_UNUSED FORCE_INLINE
    auto operator=(const RcRead& copied) -> RcRead&
    {
        Clear();

        _ptr = copied._ptr;
        _rc  = copied._rc;

        if (_rc)
            _rc->AddRead();

        return *this;
    }
};

template<typename T>
class RcWrite
{
PRIVATE:
    T*  _ptr{};
    Rc* _rc{};


    // Element Access

public:
    NO_DISCARD FORCE_INLINE
    auto HasValue() const -> bool
    {
        return _ptr != nullptr;
    }

    NO_DISCARD FORCE_INLINE
    auto Get() -> T*
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_ptr != nullptr);
        return _ptr;
    }

    NO_DISCARD FORCE_INLINE
    auto Get() const -> const T*
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_ptr != nullptr);
        return _ptr;
    }

    NO_DISCARD FORCE_INLINE
    auto operator->() -> T*
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_ptr != nullptr);
        return _ptr;
    }

    NO_DISCARD FORCE_INLINE
    auto operator->() const -> const T*
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_ptr != nullptr);
        return _ptr;
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() -> T&
    {
        return *Get();
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() const -> const T&
    {
        return *Get();
    }

    FORCE_INLINE
    void Clear()
    {
        if (_rc)
            _rc->RemoveWrite();

        _ptr = nullptr;
        _rc = nullptr;
    }


    // Lifecycle

    FORCE_INLINE
    RcWrite() = default;

    FORCE_INLINE
    RcWrite(RcWrite&& other) noexcept
    {
        ::Swap(_ptr, other._ptr);
        ::Swap(_rc, other._rc);
    }

    RcWrite(const RcWrite& copied) = delete; // Copying is not allowed.

    FORCE_INLINE
    RcWrite(Rc& rc, T& ptr)
        : _ptr{ &ptr }
        , _rc{ &rc }
    {
        _rc->AddWrite();
    }

    FORCE_INLINE
    ~RcWrite()
    {
        Clear();
    }

    MAYBE_UNUSED FORCE_INLINE
    auto operator=(RcWrite&& other) noexcept -> RcWrite&
    {
        Clear();
        ::Swap(_ptr, other._ptr);
        ::Swap(_rc, other._rc);
        return *this;
    }

    auto operator=(const RcWrite& copied)->RcWrite & = delete; // Copying is not allowed.
};
