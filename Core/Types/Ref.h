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

    
    // Tombstone

    friend TombstoneNullable<Ref>;

private:
    FORCE_INLINE
    explicit Ref(MAYBE_UNUSED TombstoneDepth tombstoneTag) noexcept
        : _value{ nullptr }
    {
        ASSERT_COLLECTION_INTEGRITY(tombstoneTag.Value == 1); // Reference does not support any other tombstone depth.
        // Puts the other ref into invalid state.
    }

    NO_DISCARD FORCE_INLINE
    bool IsTombstone() const
    {
        return _value == nullptr;
    }

    NO_DISCARD FORCE_INLINE
    int8 GetTombstoneLevel() const
    {
        return 1; // Reference does not support any other tombstone depth.
    }


    // Element Access

public:
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
struct GetMaxTombstoneDepth<Ref<T>>
{
    enum { Value = 1 };
};
