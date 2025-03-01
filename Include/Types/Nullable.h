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
#include "Types/Base.h"
#include "Types/Dummy.h"

template<typename T>
class Nullable<T, false>
{
public:
    using Element = T;

PRIVATE:
    union
    {
        Element _value;
        Dummy   _dummy; // Ensure default ctor.
    };
    int8 _nullLevel{ 1 };


    // Element Access

public:
    /// <summary> Checks if the nullable has a value. </summary>
    NO_DISCARD FORCE_INLINE
    auto HasValue() const NOEXCEPT_Y -> bool
    {
        return _nullLevel == 0;
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    auto Value() NOEXCEPT_Y -> Element&
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    auto Value() const NOEXCEPT_Y -> const Element&
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Reference to the value or the fallback. </summary>
    NO_DISCARD FORCE_INLINE
    auto ValueOr(const Element& fallback) const NOEXCEPT_Y -> const Element&
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
    void Set(Element&& value) NOEXCEPT_Y
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
    void Set(const Element& value) NOEXCEPT_Y
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
    void Emplace(Args&&... args) NOEXCEPT_Y
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
    void Clear() NOEXCEPT_Y
    {
        if (HasValue())
        {
            _value.~Element();
            _nullLevel = 1;
        }
    }


    /// <summary> Creates a span which can be used to access the value. </summary>
    template <typename U = T>
    NO_DISCARD FORCE_INLINE constexpr
    auto AsSpan() -> Span<Element>;

    /// <summary> Creates a span which can be used to access the value. </summary>
    template <typename U = T>
    NO_DISCARD FORCE_INLINE constexpr
    auto AsSpan() const -> Span<const Element>;


    // Tombstone (Nested Nullable)

    friend Nullable<Nullable, true>;

PRIVATE:
    NO_DISCARD FORCE_INLINE
    auto IsTombstone() const NOEXCEPT_Y -> bool
    {
        return _nullLevel > 1;
    }

    NO_DISCARD FORCE_INLINE
    auto GetTombstoneLevel() const NOEXCEPT_Y -> int8
    {
        return _nullLevel - 1; // Go out
    }

    /// <summary>
    /// Creates a tombstone of a nullable.
    /// You better know what you are doing, if you want to use it!
    /// </summary>
    FORCE_INLINE explicit
    Nullable(const TombstoneDepth tombstoneTag) NOEXCEPT_Y
        : _nullLevel{ static_cast<int8>(tombstoneTag.Value + 1) } // Go in
    {
        ASSERT(tombstoneTag.Value >= 0);
    }


    // Lifecycle

public:
    /// <summary> Initializes empty nullable. </summary>
    FORCE_INLINE constexpr
    Nullable() NOEXCEPT_Y
    {
        // Pass (`default` not supported)
    }

    /// <summary> Initializes empty nullable. </summary>
    FORCE_INLINE constexpr explicit
    Nullable(NullOptT) NOEXCEPT_Y
    {
        // Pass (`default` not supported)
    }

    /// <summary> Initializes nullable with a copy of the specified value. </summary>
    FORCE_INLINE
    Nullable(const Nullable& other) NOEXCEPT_Y
        : _nullLevel{ other._nullLevel }
    {
        if (HasValue())
        {
            new (&_value) Element{ other._value };
        }
    }

    /// <summary> Initializes nullable by moving the value from other nullable. </summary>
    FORCE_INLINE
    Nullable(Nullable&& other) NOEXCEPT_S
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
    auto operator=(const Nullable& other) NOEXCEPT_Y -> Nullable&
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
    auto operator=(Nullable&& other) NOEXCEPT_S -> Nullable&
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
    ~Nullable() NOEXCEPT_Y
    {
        Clear();
    }


    // Conversion

    /// <summary>
    /// Converts the nullable to boolean indicating whether the nullable has a value.
    /// </summary>
    NO_DISCARD FORCE_INLINE explicit
    operator bool() const NOEXCEPT_Y
    {
        return HasValue();
    }


    // Utility

    NO_DISCARD FORCE_INLINE
    auto IsEmpty() const NOEXCEPT_Y -> bool
    {
        return !HasValue();
    }

    /// <summary> Overwrites the value with the specified one by move, if it is null. </summary>
    FORCE_INLINE
    void SetIfNull(Element&& value) NOEXCEPT_Y
    {
        if (!HasValue())
        {
            Set(MOVE(value));
        }
    }

    /// <summary> Overwrites the value with the specified one by copy, if it is null. </summary>
    FORCE_INLINE
    void SetIfNull(const Element& value) NOEXCEPT_Y
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
    auto HasValue() const NOEXCEPT_Y -> bool
    {
        return !_value.IsTombstone(); // Use the underlying type's tombstone.
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    auto Value() NOEXCEPT_Y -> Element&
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Reference to the value. Nullable must not be empty. </summary>
    NO_DISCARD FORCE_INLINE
    auto Value() const NOEXCEPT_Y -> const Element&
    {
        ASSERT(HasValue());
        return _value;
    }

    /// <summary> Reference to the value or the fallback. </summary>
    NO_DISCARD FORCE_INLINE
    auto ValueOr(const Element& fallback) const NOEXCEPT_Y -> const Element&
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
    void Set(Element&& value) NOEXCEPT_Y
    {
        _value = MOVE(value); // Should overwrite the tombstone. (or should it?)
    }

    /// <summary> Overwrites the value with the specified one by copy. </summary>
    FORCE_INLINE
    void Set(const Element& value) NOEXCEPT_Y
    {
        _value = value; // Should overwrite the tombstone. (or should it?)
    }

    /// <summary> Overwrites the value with the specified ony by emplace (ctor only). </summary>
    /// <remarks>
    /// If you want to assign null, use <c>Clear()</c>.
    /// Never pass the tombstone depth directly.
    /// </remarks>
    template<typename... Args>
    void Emplace(Args&&... args) NOEXCEPT_Y
    {
        _value.~Element();
        new (&_value) Element{ FORWARD(Args, args)... };
    }

    /// <summary> Resets the value to null. </summary>
    FORCE_INLINE
    void Clear() NOEXCEPT_Y
    {
        _value = Element{ TombstoneDepth{ 1 } };
    }

    /// <summary> Creates a span which can be used to access the value. </summary>
    template<typename U = T>
    NO_DISCARD FORCE_INLINE constexpr
    auto AsSpan() -> Span<Element>;

    /// <summary> Creates a span which can be used to access the value. </summary>
    template<typename U = T>
    NO_DISCARD FORCE_INLINE constexpr
    auto AsSpan() const -> Span<const Element>;


    // Tombstone (Nested Nullable)

    friend Nullable<Nullable, true>;

PRIVATE:
    NO_DISCARD FORCE_INLINE
    auto IsTombstone() const NOEXCEPT_Y -> bool
    {
        return _value.IsTombstone() && GetTombstoneLevel() > 0;
    }

    NO_DISCARD FORCE_INLINE
    auto GetTombstoneLevel() const  NOEXCEPT_Y-> int8
    {
        return _value.GetTombstoneLevel() - 1; // Go out
    }

    /// <summary>
    /// Creates a tombstone of a nullable.
    /// You better know what you are doing, if you want to use it!
    /// </summary>
    FORCE_INLINE explicit
    Nullable(const TombstoneDepth tombstoneTag) NOEXCEPT_Y
        : _value{ TombstoneDepth{static_cast<int8>(tombstoneTag.Value + 1) } } // Go in
    {
        ASSERT(tombstoneTag.Value > 0);
    }


    // Lifecycle

public:
    /// <summary> Initializes empty nullable. </summary>
    FORCE_INLINE constexpr
    Nullable() NOEXCEPT_Y
    {
        // Pass (`default` not supported)
    }

    /// <summary> Initializes empty nullable. </summary>
    FORCE_INLINE constexpr explicit
    Nullable(NullOptT) NOEXCEPT_Y
    {
        // Pass (`default` not supported)
    }

    /// <summary> Initializes nullable with a copy of the specified value. </summary>
    FORCE_INLINE
    Nullable(const Nullable& other) NOEXCEPT_Y
        : _value{ other._value }
    {
    }

    /// <summary> Initializes nullable by moving the value from other nullable. </summary>
    FORCE_INLINE
    Nullable(Nullable&& other) NOEXCEPT_S
        : _value{ MOVE(other._value) }
    {
        other.Clear();
    }

    /// <summary> Assigns the value from the specified nullable. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator=(const Nullable& other) NOEXCEPT_Y -> Nullable&
    {
        if (this != &other)
        {
            _value = other._value;
        }
        return *this;
    }

    /// <summary> Assigns the value by moving it from the specified nullable. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator=(Nullable&& other) NOEXCEPT_S -> Nullable&
    {
        if (this != &other)
        {
            _value = MOVE(other._value);
            other.Clear();
        }
        return *this;
    }

    FORCE_INLINE
    ~Nullable() NOEXCEPT_Y
    {
        Clear();
    }


    // Conversion

    /// <summary>
    /// Converts the nullable to boolean indicating whether the nullable has a value.
    /// </summary>
    NO_DISCARD FORCE_INLINE explicit
    operator bool() const NOEXCEPT_Y
    {
        return HasValue();
    }


    // Utility

    NO_DISCARD FORCE_INLINE
    auto IsEmpty() const NOEXCEPT_Y -> bool
    {
        return !HasValue();
    }

    /// <summary> Overwrites the value with the specified one by move, if it is null. </summary>
    FORCE_INLINE
    void SetIfNull(Element&& value) NOEXCEPT_Y
    {
        if (!HasValue())
        {
            Set(MOVE(value));
        }
    }

    /// <summary> Overwrites the value with the specified one by copy, if it is null. </summary>
    FORCE_INLINE
    void SetIfNull(const Element& value) NOEXCEPT_Y
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
/// Creates a nullable with the specified value by move.
/// <summary>
/// <remarks>
/// This method replaces constructors to allow for trivial initialization.
/// </remarks>
template<typename T>
NO_DISCARD FORCE_INLINE
auto MakeNullable(T&& value) NOEXCEPT_Y
{
    using Element = typename TRemoveRef<T>::Type;
    Nullable<Element> result{};
    result.Set(FORWARD(T, value));
    return result;
}

/// <summary>
/// Creates a nullable with no value.
/// <summary>
/// <remarks>
/// This method replaces constructors to allow for trivial initialization.
/// </remarks>
template<typename T>
NO_DISCARD FORCE_INLINE
auto MakeNullable(NullOptT) NOEXCEPT_Y -> Nullable<T>
{
    return Nullable<T>{};
}
