// Created by Mateusz Karbowiak 2024

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

private:
    Element* _value;
    

    // Lifecycle

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
    explicit Ref(TombstoneTag) noexcept
        : _value{ nullptr }
    {
        // Puts the other ref into invalid state.
    }

    FORCE_INLINE
    Ref& operator=(const Ref& other)
    {
        _value = other._value;
        return *this;
    }

    FORCE_INLINE
    Ref& operator=(Ref&& other) noexcept
    {
        _value = other._value;
        other._value = nullptr;
        // Puts the other ref into invalid state.
        return *this;
    }

    FORCE_INLINE
    ~Ref() = default;


    // Element Access

    NO_DISCARD FORCE_INLINE
    Element& Value() const
    {
        return *_value;
    }

    NO_DISCARD FORCE_INLINE
    Element* Get() const
    {
        return _value;
    }

    NO_DISCARD FORCE_INLINE
    Element& operator*() const
    {
        return Value();
    }

    NO_DISCARD FORCE_INLINE
    Element* operator->() const
    {
        return _value;
    }


    // Tombstone

    NO_DISCARD FORCE_INLINE
    bool IsTombstone() const
    {
        return _value == nullptr;
    }


    // Comparison

    NO_DISCARD FORCE_INLINE
    bool operator==(const Ref& other) const
    {
        return _value == other._value;
    }

    NO_DISCARD FORCE_INLINE
    bool operator!=(const Ref& other) const
    {
        return _value != other._value;
    }

    NO_DISCARD FORCE_INLINE
    bool operator<(const Ref& otherTag) const
    {
        return _value < otherTag._value;
    }
};

template<typename T>
struct IsTombstoneSupported<Ref<T>>
{
    static constexpr bool Value = true;
};
