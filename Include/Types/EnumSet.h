// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include <Language/TypeTraits.h>
#include <Debugging/Assertions.h>

/// <summary>
/// Set of enum classes, stored as a bitfield.
/// </summary>
/// 
/// <typeparam name="E"> 
/// Enum class type to be used for bitfield indexing.
/// </typeparam>
/// <typeparam name="S"> 
/// (Optional) Underlying type storing the bitfield. 
/// Can be used if only a part of the enum class is needed.
/// </typeparam>
template<typename E, typename S = typename TUnderlyingType<E>::Type>
class EnumSet
{
public:
    using EnumType    = E;
    using StorageType = S;

PRIVATE:
    StorageType _value = 0;

    static_assert(TIsEnumV<EnumType>, "EnumSet can only be used with enum classes.");


    // Set operations

public:
    /// <summary> Adds an enum value to the set. </summary>
    MAY_DISCARD FORCE_INLINE
    auto Add(const EnumType value) NOEXCEPT_Y -> EnumSet&
    {
        _value |= StorageType{ 1 } << static_cast<int32>(value);
        return *this;
    }

    /// <summary> Removes an enum value from the set. </summary>
    MAY_DISCARD FORCE_INLINE
    auto Remove(const EnumType value) NOEXCEPT_Y -> EnumSet&
    {
        _value &= ~(StorageType{ 1 } << static_cast<int32>(value));
        return *this;
    }

    /// <summary> Toggles an enum value in the set. </summary>
    MAY_DISCARD FORCE_INLINE
    auto Flip(const EnumType value) NOEXCEPT_Y -> EnumSet&
    {
        _value ^= StorageType{ 1 } << static_cast<int32>(value);
        return *this;
    }

    /// <summary> Checks if the set contains an enum value. </summary>
    NO_DISCARD FORCE_INLINE
    auto Contains(const EnumType value) const NOEXCEPT_Y -> bool
    {
        return (_value & (StorageType{ 1 } << static_cast<int32>(value))) != 0;
    }

    /// <summary> Checks if the set is empty. </summary>
    NO_DISCARD FORCE_INLINE
    auto IsEmpty() const NOEXCEPT_Y -> bool
    {
        return _value == 0;
    }

    /// <summary> Clears the set. </summary>
    MAY_DISCARD FORCE_INLINE
    auto Clear() NOEXCEPT_Y -> EnumSet&
    {
        _value = 0;
        return *this;
    }

    /// <summary> Counts the number of enum values in the set. </summary>
    NO_DISCARD FORCE_INLINE
    auto Count() const NOEXCEPT_Y -> int32
    {
        const uint64 valueAsQword = static_cast<uint64>(_value);
        return POP_COUNT64(valueAsQword);
    }

    /// <summary> Gets the raw storage value. </summary>
    auto Raw() const NOEXCEPT_Y -> StorageType
    {
        return _value;
    }


    // Identity

    /// <summary> Checks if the set is equal to another set. </summary>
    NO_DISCARD FORCE_INLINE
    auto operator==(const EnumSet& other) const NOEXCEPT_Y -> bool
    {
        return _value == other._value;
    }

    /// <summary> Checks if the set is not equal to another set. </summary>
    NO_DISCARD FORCE_INLINE
    auto operator!=(const EnumSet& other) const NOEXCEPT_Y -> bool
    {
        return _value != other._value;
    }

    /// <summary> Checks if the set is a subset of another set. </summary>
    NO_DISCARD FORCE_INLINE
    auto operator<=(const EnumSet& other) const NOEXCEPT_Y -> bool
    {
        return (_value & other._value) == _value;
    }


    // Lifecycle

    /// <summary> Default constructor (empty set). </summary>
    FORCE_INLINE
    EnumSet() = default;

    /// <summary> Construct with a single enum value. </summary>
    FORCE_INLINE explicit 
    EnumSet(const EnumType value) NOEXCEPT_Y
    {
        Add(value);
    }

    /// <summary> Construct from raw storage value. </summary>
    FORCE_INLINE explicit 
    EnumSet(const StorageType value) NOEXCEPT_Y
        : _value(value)
    {
    }



    // Bitwise operations

    /// <summary> Bitwise OR assignment with another EnumSet. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator|=(const EnumSet& other) NOEXCEPT_Y -> EnumSet&
    {
        _value |= other._value;
        return *this;
    }

    /// <summary> Bitwise AND assignment with another EnumSet. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator&=(const EnumSet& other) NOEXCEPT_Y -> EnumSet&
    {
        _value &= other._value;
        return *this;
    }

    /// <summary> Bitwise XOR assignment with another EnumSet. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator^=(const EnumSet& other) NOEXCEPT_Y -> EnumSet&
    {
        _value ^= other._value;
        return *this;
    }

    /// <summary> Bitwise OR operator. </summary>
    NO_DISCARD FORCE_INLINE
    auto operator|(const EnumSet& other) const NOEXCEPT_Y -> EnumSet
    {
        EnumSet result = *this;
        result |= other;
        return result;
    }

    /// <summary> Bitwise AND operator. </summary>
    NO_DISCARD FORCE_INLINE
    auto operator&(const EnumSet& other) const NOEXCEPT_Y -> EnumSet
    {
        EnumSet result = *this;
        result &= other;
        return result;
    }

    /// <summary> Bitwise XOR operator. </summary>
    NO_DISCARD FORCE_INLINE
    auto operator^(const EnumSet& other) const NOEXCEPT_Y -> EnumSet
    {
        EnumSet result = *this;
        result ^= other;
        return result;
    }

    /// <summary> Bitwise NOT operator. </summary>
    NO_DISCARD FORCE_INLINE
    auto operator~() const NOEXCEPT_Y -> EnumSet
    {
        EnumSet result;
        result._value = ~_value;
        return result;
    }
};
