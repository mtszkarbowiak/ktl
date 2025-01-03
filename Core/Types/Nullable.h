// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"
#include "Language/Templates.h"

/// <summary>
/// Wraps a value type so that it can be assigned null.
/// The value is stored in the nullable object itself.
/// </summary>
/// <typeparam name="T"> Type of the stored value. </typeparam>
template<typename T>
class Nullable
{
public:
    using Element = T;

private:
    // Storage

    /// <summary>
    /// Nullable value container that contains a boolean value or null.
    /// </summary>
    class FlagStorage final
    {
        Element _value;
        bool _hasValue{ false };


    public:
        NO_DISCARD FORCE_INLINE
        bool HasValue() const
        {
            return _hasValue;
        }
        
        FORCE_INLINE
        void Reset()
        {
            if (_hasValue)
            {
                _value.~T();
                _hasValue = false;
            }
        }

        template<typename... Args>
        FORCE_INLINE
        void Emplace(Args&&... args)
        {
            if (_hasValue) 
            {
                _value = Element{ FORWARD(Args, args)... };
            }
            else 
            {
                new (&_value) Element{ FORWARD(Args, args)... };
                _hasValue = true;
            }
        }

        NO_DISCARD FORCE_INLINE
        Element& Value()
        {
            return _value;
        }

        NO_DISCARD FORCE_INLINE
        const Element& Value() const
        {
            return _value;
        }
    };

    /// <summary>
    /// Nullable value container that uses a tombstone value to represent null.
    /// </summary>
    class TombstoneStorage final
    {
        Element _value{ TombstoneDepth{ 1 } }; // Currently only the first tombstone level is supported.

    public:
        NO_DISCARD FORCE_INLINE
        bool HasValue() const
        {
            return !_value.IsTombstone();
        }

        FORCE_INLINE
        void Reset()
        {
            if (!_value.IsTombstone())
            {
                _value = Element{ TombstoneDepth{ 1 } };
            }
        }

        template<typename... Args>
        FORCE_INLINE
        void Emplace(Args&&... args)
        {
            //TODO Assertion that TombstoneTag is not used.
            _value = Element{ FORWARD(Args, args)... };
        }

        NO_DISCARD FORCE_INLINE
        Element& Value()
        {
            return _value;
        }

        NO_DISCARD FORCE_INLINE
        const Element& Value() const
        {
            return _value;
        }
    };

public:
    /// <summary>
    /// Indicates whether the type supports a tombstone value to represent null.
    /// </summary>
    static constexpr bool HasTombstone = GetMaxTombstoneDepth<T>::Value;

private:
    using Storage = std::conditional_t<
        HasTombstone, 
        TombstoneStorage, 
        FlagStorage
    >;

    Storage _storage{};


    // Element Access

public:
    NO_DISCARD FORCE_INLINE
    bool HasValue() const
    {
        return _storage.HasValue();
    }

    /// <summary>
    /// Returns a reference to the stored value. Make sure to check if the value is present.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    const Element& Value() const
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_storage.HasValue());
        return _storage.Value();
    }

    /// <summary>
    /// Returns a reference to the stored value or the fallback value if the value is not present.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    const Element& ValueOr(const Element& fallback) const
    {
        if (_storage.HasValue())
        {
            return _storage.Value();
        }
        else 
        {
            return fallback;
        }
    }

    /// <summary>
    /// Returns a reference to the stored value or the fallback value if the value is not present.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    Element& ValueOr(Element& fallback) const
    {
        if (_storage.HasValue())
        {
            return _storage.Value();
        }
        else
        {
            return fallback;
        }
    }


    // Element Manipulation
    
    /// <summary>
    /// Sets the stored value to null, if it is present.
    /// </summary>
    FORCE_INLINE
    void Reset()
    {
        _storage.Reset();
    }

    /// <summary>
    /// Assigns a new value to the nullable.
    /// </summary>
    template<typename... Args>
    FORCE_INLINE
    void Emplace(Args&&... args)
    {
        _storage.Emplace(FORWARD(Args, args)...);
    }


    // Lifecycle

    /// <summary>
    /// Creates a new empty nullable.
    /// </summary>
    Nullable() = default;


    Nullable(Nullable&&) = default;

    Nullable(const Nullable&) = default;

    explicit Nullable(Element&& value)
    {
        Emplace(MOVE(value));
    }


    Nullable& operator=(Nullable&&) = default;

    Nullable& operator=(const Nullable&) = default;

    Nullable& operator=(Element&& value)
    {
        Emplace(MOVE(value));
        return *this;
    }


    ~Nullable()
    {
        _storage.Reset();
    }
};
