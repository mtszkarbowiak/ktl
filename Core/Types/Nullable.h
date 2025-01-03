// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"
#include "Language/Templates.h"

template<typename T, bool UseTombstone = (GetMaxTombstoneDepth<T>::Value > 0)>
class Nullable;

/// <summary>
/// Wrapper over a value type that can be assigned an additional null value.
/// To represent null, this implementation always uses a sentinel value taking additional byte.
/// </summary>
///
/// <typeparam name="T">
/// Type of the stored value. It need not to use a tombstone value.
/// </typeparam>
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
    /// <summary> Checks if the nullable has a value. </summary>
    NO_DISCARD FORCE_INLINE
    bool HasValue() const
    {
        return _nullLevel == 0;
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    Element& Value()
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    const Element& Value() const
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Overwrites the value with the specified one. </summary>
    void Set(Element&& value)
    {
        if (HasValue())
        {
            _value = MOVE(value);
        }
        else 
        {
            new (&_value) Element{ MOVE(value) };
            _nullLevel = 0;
        }
    }

    /// <summary> Resets the value to null. </summary>
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


/// <summary>
/// Wrapper over a value type that can be assigned an additional null value.
/// This implementation cedes tracking of null value to the underlying type via tombstone values.
/// Therefore, it does not require any additional memory to store the null value.
/// </summary>
/// 
/// <typeparam name="T">
/// Type of the stored value. It must support tombstone values.
/// </typeparam>
///
/// <remarks>
/// 1. 
/// </remarks>
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
    /// <summary> Checks if the nullable has a value. </summary>
    NO_DISCARD FORCE_INLINE
    bool HasValue() const
    {
        return !_value.IsTombstone(); // Use the underlying type's tombstone.
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    Element& Value()
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    const Element& Value() const
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Overwrites the value with the specified one. </summary>
    void Set(Element&& value)
    {
        _value = MOVE(value); // Should overwrite the tombstone. (or should it?)
    }

    /// <summary> Resets the value to null. </summary>
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


/// <summary>
/// Type alias for a nullable type that enforces usage of sentinel value.
/// </summary>
template<typename T>
using SentinelNullable = Nullable<T, false>;

/// <summary>
/// Type alias for a nullable type that enforces usage of tombstone value.
/// All constraints of the underlying type must be met.
/// </summary>
template<typename T>
using TombstoneNullable = Nullable<T, true>;
