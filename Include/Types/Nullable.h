// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Language/Templates.h"

/// <summary>
/// Wrapper over a value type that can be assigned an additional null value.
/// </summary>
///
/// <typeparam name="T">
/// Type of the stored value.
/// </typeparam>
/// <typeparam name="UseTombstone">
/// Flag indicating whether to use tombstone value.
/// </typeparam>
template<typename T, bool M = (GetMaxTombstoneDepth<T>::Value > 0)>
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

PRIVATE:
    union
    {
        Element _value;
    };
    int8 _nullLevel{ 1 };


    // Element Access

public:
    /// <summary> Checks if the nullable has a value. </summary>
    NO_DISCARD FORCE_INLINE
    auto HasValue() const -> bool
    {
        return _nullLevel == 0;
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    auto Value() -> Element&
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    auto Value() const -> const Element&
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Reference to the value or the fallback. </summary>
    NO_DISCARD FORCE_INLINE
    auto ValueOr(const Element& fallback) const -> const Element&
    {
        if (HasValue())
        {
            return _value;
        }
        else 
        {
            return fallback;
        }
    }

    /// <summary> Overwrites the value with the specified one by move. </summary>
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

    /// <summary> Overwrites the value with the specified one by copy. </summary>
    void Set(const Element& value)
    {
        if (HasValue())
        {
            _value = value;
        }
        else
        {
            new (&_value) Element{ value };
            _nullLevel = 0;
        }
    }

    /// <summary> Overwrites the value with the specified ony by emplace (ctor only). </summary>
    template<typename... Args>
    void Emplace(Args&&... args)
    {
        if (HasValue())
        {
            _value.~Element();
            new (&_value) Element{ FORWARD(Args, args)... };
        }
        else
        {
            new (&_value) Element{ FORWARD(Args, args)... };
            _nullLevel = 0;
        }
    }


    /// <summary> Resets the value to null. </summary>
    void Clear()
    {
        if (HasValue())
        {
            _value.~Element();
            _nullLevel = 1;
        }
    }


    // Tombstone (Nested Nullable)

    friend Nullable<Nullable, true>;

PRIVATE:
    NO_DISCARD FORCE_INLINE
    auto IsTombstone() const -> bool
    {
        return _nullLevel > 1;
    }

    NO_DISCARD FORCE_INLINE
    auto GetTombstoneLevel() const -> int8
    {
        return _nullLevel - 1; // Go out
    }

    /// <summary>
    /// Creates a tombstone of a nullable.
    /// You better know what you are doing, if you want to use it!
    /// </summary>
    FORCE_INLINE explicit
    Nullable(const TombstoneDepth tombstoneTag)
        : _nullLevel{ static_cast<int8>(tombstoneTag.Value + 1) } // Go in
    {
        ASSERT(tombstoneTag.Value >= 0);
    }


    // Lifecycle

public:
    /// <summary> Initializes empty nullable. </summary>
    FORCE_INLINE constexpr
    Nullable()
    {
        // Pass (`default` not supported)
    }

    /// <summary> Initializes nullable with the specified value. </summary>
    FORCE_INLINE explicit
    Nullable(Element&& value)
        : _value{ MOVE(value) }
        , _nullLevel{ 0 }
    {
    }

    /// <summary> Initializes nullable with a copy of the specified value. </summary>
    FORCE_INLINE
    Nullable(const Nullable& other)
        : _nullLevel{ other._nullLevel }
    {
        if (HasValue())
        {
            new (&_value) Element{ other._value };
        }
    }

    /// <summary> Initializes nullable by moving the value from other nullable. </summary>
    FORCE_INLINE
    Nullable(Nullable&& other) noexcept
        : _nullLevel{ other._nullLevel }
    {
        if (HasValue())
        {
            new (&_value) Element{ MOVE(other._value) };
            other.Clear();
        }
    }

    /// <summary> Assigns the value from the specified nullable. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator=(const Nullable& other) -> Nullable&
    {
        if (this != &other)
        {
            Clear();
            _nullLevel = other._nullLevel;
            if (HasValue())
            {
                new (&_value) Element{ other._value };
            }
        }
        return *this;
    }

    /// <summary> Assigns the value by moving it from the specified nullable. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator=(Nullable&& other) noexcept -> Nullable&
    {
        if (this != &other)
        {
            Clear();
            _nullLevel = other._nullLevel;
            if (HasValue())
            {
                new (&_value) Element{ MOVE(other._value) };
                other.Clear();
            }
        }
        return *this;
    }

    FORCE_INLINE
    ~Nullable()
    {
        Clear();
    }


    // Conversion

    /// <summary>
    /// Converts the nullable to boolean indicating whether the nullable has a value.
    /// </summary>
    NO_DISCARD FORCE_INLINE explicit
    operator bool() const
    {
        return HasValue();
    }


    // Utility

    NO_DISCARD FORCE_INLINE
    auto IsEmpty() const -> bool
    {
        return !HasValue();
    }

    /// <summary> Overwrites the value with the specified one by move, if it is null. </summary>
    FORCE_INLINE
    void SetIfNull(Element&& value)
    {
        if (!HasValue())
        {
            Set(MOVE(value));
        }
    }

    /// <summary> Overwrites the value with the specified one by copy, if it is null. </summary>
    FORCE_INLINE
    void SetIfNull(const Element& value)
    {
        if (!HasValue())
        {
            Set(value);
        }
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
/// Additionally, the underlying type may skip the null value check.
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

PRIVATE:
    Element _value{ TombstoneDepth{ 1 } };


    // Element Access

public:
    /// <summary> Checks if the nullable has a value. </summary>
    NO_DISCARD FORCE_INLINE
    auto HasValue() const -> bool
    {
        return !_value.IsTombstone(); // Use the underlying type's tombstone.
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    auto Value() -> Element&
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    auto Value() const -> const Element&
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Reference to the value or the fallback. </summary>
    NO_DISCARD FORCE_INLINE
    auto ValueOr(const Element& fallback) const -> const Element&
    {
        if (HasValue())
        {
            return _value;
        }
        else 
        {
            return fallback;
        }
    }

    /// <summary> Overwrites the value with the specified one by move. </summary>
    FORCE_INLINE
    void Set(Element&& value)
    {
        _value = MOVE(value); // Should overwrite the tombstone. (or should it?)
    }

    /// <summary> Overwrites the value with the specified one by copy. </summary>
    FORCE_INLINE
    void Set(const Element& value)
    {
        _value = value; // Should overwrite the tombstone. (or should it?)
    }

    /// <summary> Overwrites the value with the specified ony by emplace (ctor only). </summary>
    /// <remarks>
    /// If you want to assign null, use <c>Clear()</c>.
    /// Never pass the tombstone depth directly.
    /// </remarks>
    template<typename... Args>
    void Emplace(Args&&... args)
    {
        _value.~Element();
        new (&_value) Element{ FORWARD(Args, args)... };
    }

    /// <summary> Resets the value to null. </summary>
    FORCE_INLINE
    void Clear()
    {
        _value = Element{ TombstoneDepth{ 1 } };
    }


    // Tombstone (Nested Nullable)

    friend Nullable<Nullable, true>;

PRIVATE:
    NO_DISCARD FORCE_INLINE
    auto IsTombstone() const -> bool
    {
        return GetTombstoneLevel() > 0;
    }

    NO_DISCARD FORCE_INLINE
    auto GetTombstoneLevel() const -> int8
    {
        return _value.GetTombstoneLevel() - 1; // Go out
    }

    /// <summary>
    /// Creates a tombstone of a nullable.
    /// You better know what you are doing, if you want to use it!
    /// </summary>
    FORCE_INLINE explicit
    Nullable(const TombstoneDepth tombstoneTag)
        : _value{ TombstoneDepth{static_cast<int8>(tombstoneTag.Value + 1) } } // Go in
    {
        ASSERT(tombstoneTag.Value > 0);
    }


    // Lifecycle

public:
    /// <summary> Initializes empty nullable. </summary>
    FORCE_INLINE constexpr
    Nullable()
    {
        // Pass (`default` not supported)
    }

    /// <summary> Initializes nullable with the specified value. </summary>
    FORCE_INLINE explicit
    Nullable(Element&& value)
        : _value{ MOVE(value) }
    {
    }

    /// <summary> Initializes nullable with a copy of the specified value. </summary>
    FORCE_INLINE
    Nullable(const Nullable& other)
        : _value{ other._value }
    {
    }

    /// <summary> Initializes nullable by moving the value from other nullable. </summary>
    FORCE_INLINE
    Nullable(Nullable&& other) noexcept
        : _value{ MOVE(other._value) }
    {
        other.Clear();
    }

    /// <summary> Assigns the value from the specified nullable. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator=(const Nullable& other) -> Nullable&
    {
        if (this != &other)
        {
            _value = other._value;
        }
        return *this;
    }

    /// <summary> Assigns the value by moving it from the specified nullable. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator=(Nullable&& other) noexcept -> Nullable&
    {
        if (this != &other)
        {
            _value = MOVE(other._value);
            other.Clear();
        }
        return *this;
    }

    FORCE_INLINE
    ~Nullable()
    {
        Clear();
    }


    // Conversion

    /// <summary>
    /// Converts the nullable to boolean indicating whether the nullable has a value.
    /// </summary>
    NO_DISCARD FORCE_INLINE explicit
    operator bool() const
    {
        return HasValue();
    }


    // Utility

    NO_DISCARD FORCE_INLINE
    auto IsEmpty() const -> bool
    {
        return !HasValue();
    }

    /// <summary> Overwrites the value with the specified one by move, if it is null. </summary>
    FORCE_INLINE
    void SetIfNull(Element&& value)
    {
        if (!HasValue())
        {
            Set(MOVE(value));
        }
    }

    /// <summary> Overwrites the value with the specified one by copy, if it is null. </summary>
    FORCE_INLINE
    void SetIfNull(const Element& value)
    {
        if (!HasValue())
        {
            Set(value);
        }
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
