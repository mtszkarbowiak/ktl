// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"
#include "Language/Templates.h"

/// <summary>
/// Wraps a value type so that it can be assigned null.
/// </summary>
/// <typeparam name="T"> Type of the stored value. </typeparam>
template<typename T>
class Nullable final
{
    /// <summary>
    /// Nullable value container that contains a boolean value or null.
    /// </summary>
    class FlagStorage final
    {
        T _value;
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
                _value = T{ FORWARD(Args, args)... };
            }
            else 
            {
                new (&_value) T{ FORWARD(Args, args)... };
                _hasValue = true;
            }
        }

        NO_DISCARD FORCE_INLINE
        T& Value()
        {
            return _value;
        }

        NO_DISCARD FORCE_INLINE
        const T& Value() const
        {
            return _value;
        }
    };

    /// <summary>
    /// Nullable value container that uses a tombstone value to represent null.
    /// </summary>
    class TombstoneStorage final
    {
        T _value{ TombstoneTag{} };

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
                _value = T{ TombstoneTag{} };
            }
        }

        template<typename... Args>
        FORCE_INLINE
        void Emplace(Args&&... args)
        {
            _value = T{ FORWARD(Args, args)... };
        }

        NO_DISCARD FORCE_INLINE
        T& Value()
        {
            return _value;
        }

        NO_DISCARD FORCE_INLINE
        const T& Value() const
        {
            return _value;
        }
    };

public:
    /// <summary>
    /// Indicates whether the type supports a tombstone value to represent null.
    /// </summary>
    static constexpr bool HasTombstone = IsTombstoneSupported<T>::Value;

private:
    using Storage = std::conditional_t<
        HasTombstone, 
        TombstoneStorage, 
        FlagStorage
    >;

    Storage _storage{};

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
    const T& Value() const
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_storage.HasValue());
        return _storage.Value();
    }

    /// <summary>
    /// Returns a reference to the stored value or the fallback value if the value is not present.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    const T& ValueOr(const T& fallback) const
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


    /// <summary>
    /// Creates a new empty nullable.
    /// </summary>
    Nullable() = default;

    /// <summary>
    /// Creates a nullable with a value created from the provided arguments.
    /// </summary>
    template<typename... Args>
    explicit Nullable(Args&&... args)
    {
        Emplace(FORWARD(Args, args)...);
    }

    /// <summary>
    /// Assigns a new value to the nullable.
    /// </summary>
    template<typename... Args>
    Nullable& operator=(Args&&... args)
    {
        Emplace(FORWARD(Args, args)...);
        return *this;
    }
};
