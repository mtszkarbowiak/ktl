// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/Keywords.h"
#include "Language/Templates.h"

/// <summary>
/// Wraps a pointer to a non-null value with support for tombstone.
/// Effectively creates a non-owning view to one value.
/// </summary>
template<typename T>
class Ref 
{
public:
    using Element = T;

PRIVATE:
    Element* _value;
    

    // Lifecycle //TODO Put lifecycle in the end.

public:
    FORCE_INLINE
    Ref() = delete;

    FORCE_INLINE
    Ref(Element& value)
        : _value{ &value }
    {
    }

    FORCE_INLINE
    Ref(const Ref& other)
        : _value{ other._value }
    {
    }

    FORCE_INLINE
    Ref(Ref&& other) noexcept
        : _value{ other._value }
    {
        other._value = nullptr;
        // Puts the other ref into invalid state.
    }

    FORCE_INLINE
    auto operator=(const Ref& other) -> Ref&
    {
        _value = other._value;
        return *this;
    }

    FORCE_INLINE
    auto operator=(Ref&& other) noexcept -> Ref&
    {
        _value = other._value;
        other._value = nullptr;
        // Puts the other ref into invalid state.
        return *this;
    }

    FORCE_INLINE
    ~Ref() = default;

    
    // Tombstone

    friend TombstoneNullable<Ref>;

PRIVATE:
    FORCE_INLINE explicit
    Ref(MAYBE_UNUSED TombstoneDepth tombstoneTag) noexcept
        : _value{ nullptr }
    {
        ASSERT_COLLECTION_INTEGRITY(tombstoneTag.Value == 1); // Reference does not support any other tombstone depth.
        // Puts the other ref into invalid state.
    }

    NO_DISCARD FORCE_INLINE
    auto IsTombstone() const -> bool
    {
        return _value == nullptr;
    }

    NO_DISCARD FORCE_INLINE
    auto GetTombstoneLevel() const -> int8
    {
        return 1; // Reference does not support any other tombstone depth.
    }


    // Element Access

public:
    NO_DISCARD FORCE_INLINE
    auto Value() const -> Element&
    {
        return *_value;
    }

    NO_DISCARD FORCE_INLINE
    auto Get() const -> Element*
    {
        return _value;
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() const -> Element&
    {
        return Value();
    }

    NO_DISCARD FORCE_INLINE
    auto operator->() const -> Element*
    {
        return _value;
    }


    // Comparison

    NO_DISCARD FORCE_INLINE
    auto operator==(const Ref& other) const -> bool
    {
        return _value == other._value;
    }

    NO_DISCARD FORCE_INLINE
    auto operator!=(const Ref& other) const -> bool
    {
        return _value != other._value;
    }

    NO_DISCARD FORCE_INLINE
    auto operator<(const Ref& otherTag) const -> bool
    {
        return _value < otherTag._value;
    }
};

template<typename T>
struct GetMaxTombstoneDepth<Ref<T>>
{
    enum { Value = 1 };
};
