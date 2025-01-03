// Created by Mateusz Karbowiak 2024

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Language/Templates.h"
#include "Types/Numbers.h"

/// <summary>
/// Wrapper over an integer value that represents an index in a collection,
/// which supports tombstone value to represent null.
/// </summary>
/// <remarks>
/// Remember that a tombstone is not a null value! Never operate directly on a tombstone.
/// </remarks>
class Index 
{
    int32 _value{};

public:
    /// <summary>
    /// Creates index of default value, which is zero.
    /// </summary>
    FORCE_INLINE
    Index() = default;

    /// <summary>
    /// Creates an index with the specified value.
    /// </summary>
    FORCE_INLINE
    Index(const int32 value)
        : _value{ value }
    {
        ASSERT(value >= 0); // Value must be non-negative. To create a tombstone, use Nullable.
    }


    // Conversion

    /// <summary>
    /// Implicit conversion to integer.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    operator int32() const
    {
        ASSERT_COLLECTION_SAFE_ACCESS(!IsTombstone()); // Accessing a tombstone is strictly forbidden.
        return _value;
    }


    // Tombstone

    /// <summary>
    /// Creates a tombstone of an index.
    /// Do not use this constructor directly, use <c>Nullable</c> instead.
    /// </summary>
    FORCE_INLINE
    explicit Index(TombstoneDepth tombstoneTag)
        : _value{ -tombstoneTag.Value }
    {
        ASSERT(tombstoneTag.Value > 0); // Tombstone depth must be greater than zero.
    }

    /// <summary>
    /// Checks if the index is a tombstone.
    /// Do not use this method directly, use <c>Nullable</c> instead.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    bool IsTombstone() const
    {
        return _value < 0;
    }

    NO_DISCARD FORCE_INLINE
    int8 GetTombstoneLevel() const
    {
        return static_cast<int8>(-_value);
    }
};

template<>
struct GetMaxTombstoneDepth<Index>
{
    enum { Value = 64 }; // More than enough for any collection.
};
