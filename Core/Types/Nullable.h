// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"
#include "Language/Templates.h"

/// <summary>
/// Wraps a value type so that it can be assigned null.
/// The value is stored in the nullable object itself.
/// </summary>
/// <typeparam name="T"> Type of the stored value. </typeparam>
template<typename T, bool UseTombstone = (GetMaxTombstoneDepth<T>::Value > 0)>
class Nullable;


/// <summary> Default implementation of nullable type using a sentinel value. </summary>
template<typename T>
class Nullable<T, false>
{
public:
    using Element = T;

private:
    Element _value;
    int8 _nullLevel{ 1 };


    // Element Access

public:
    NO_DISCARD FORCE_INLINE
    bool HasValue() const
    {
        return _nullLevel == 0;
    }

    NO_DISCARD FORCE_INLINE
    Element& Value()
    {
        ASSERT(HasValue());
        return _value;
    }

    NO_DISCARD FORCE_INLINE
    const Element& Value() const
    {
        ASSERT(HasValue());
        return _value;
    }

    void Set(Element&& value)
    {
        _value = MOVE(value);
        _nullLevel = 0;
    }

    void Reset()
    {
        if (HasValue())
        {
            _value.~Element();
            _nullLevel = 0;
        }
    }


    // Tombstone (Nested Nullable)

    NO_DISCARD FORCE_INLINE
    bool IsTombstone() const
    {
        return _nullLevel > 1;
    }

    NO_DISCARD FORCE_INLINE
    int8 GetTombstoneLevel() const
    {
        return _nullLevel - 1; // Go out
    }

    explicit Nullable(const TombstoneDepth tombstoneTag)
        : _nullLevel{ static_cast<int8>(tombstoneTag.Value + 1) } // Go in
    {
        ASSERT(tombstoneTag.Value >= 0);
    }


    // Lifecycle

    Nullable() = default;

    explicit Nullable(Element&& value)
        : _value{ MOVE(value) }
        , _nullLevel{ 0 }
    {
    }

    ~Nullable()
    {
        Reset();
    }
};

template<typename T>
struct GetMaxTombstoneDepth<Nullable<T, false>>
{
    enum { Value = 64 };
};


/// <summary> Alternative implementation of nullable type using a tombstone value. </summary>
template<typename T>
class Nullable<T, true>
{
    static_assert(GetMaxTombstoneDepth<T>::Value > 0, "Type does not support tombstone values.");

public:
    using Element = T;

private:
    Element _value{ TombstoneDepth{ 1 } };


    // Element Access

public:
    NO_DISCARD FORCE_INLINE
    bool HasValue() const
    {
        return !_value.IsTombstone(); // Use the underlying type's tombstone.
    }

    NO_DISCARD FORCE_INLINE
    Element& Value()
    {
        ASSERT(HasValue());
        return _value;
    }

    NO_DISCARD FORCE_INLINE
    const Element& Value() const
    {
        ASSERT(HasValue());
        return _value;
    }

    void Set(Element&& value)
    {
        _value = MOVE(value); // Should overwrite the tombstone. (or should it?)
    }

    void Reset()
    {
        _value = Element{ TombstoneDepth{ 1 } };
    }


    // Tombstone (Nested Nullable)

    NO_DISCARD FORCE_INLINE
    bool IsTombstone() const
    {
        return GetTombstoneLevel() > 0;
    }

    NO_DISCARD FORCE_INLINE
    int8 GetTombstoneLevel() const
    {
        return _value.GetTombstoneLevel() - 1; // Go out
    }

    explicit Nullable(const TombstoneDepth tombstoneTag)
        : _value{ TombstoneDepth{static_cast<int8>(tombstoneTag.Value + 1) } } // Go in
    {
        ASSERT(tombstoneTag.Value > 0);
    }


    // Lifecycle

    Nullable() = default;

    explicit Nullable(Element&& value)
        : _value{ MOVE(value) }
    {
    }

    ~Nullable()
    {
        Reset();
    }
};

template<typename T>
struct GetMaxTombstoneDepth<Nullable<T, true>>
{
    enum { Value = GetMaxTombstoneDepth<T>::Value - 1 };
};
