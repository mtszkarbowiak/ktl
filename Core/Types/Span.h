// Created by Mateusz Karbowiak 2024

#pragma once

#include "Types/Numbers.h"

/// <summary>
/// Non-owning view of a contiguous sequence of elements.
/// </summary>
/// <remarks>
/// Span is classified as null if it points to <c>null</c> OR its points to zero elements.
/// <remarks>
template<typename T>
class Span
{
    T*    _data{};
    int32 _count{};


    // Constructors

public:
    /// <summary> Initializes an empty span. </summary>
    FORCE_INLINE
    Span() = default;

    /// <summary> Initializes a span with the specified data and size. </summary>
    FORCE_INLINE
    Span(T* data, const int32 size)
        : _data{ data }
        , _count{ size }
    {
        ASSERT_COLLECTION_SAFE_MOD(size >= 0); // Span size must be non-negative.
        ASSERT_COLLECTION_SAFE_MOD((size == 0) || (data != nullptr)); // Span data must be valid if the size is non-zero.
    }

    /// <summary> Initializes a span between the specified begin and end pointers. </summary>
    FORCE_INLINE
    Span(T* begin, T* end)
        : _data{ begin }
        , _count{ static_cast<int32>(end - begin) }
    {
        ASSERT_COLLECTION_SAFE_MOD(begin <= end); // Span begin must be before the end.
    }

    /// <summary> Initializes a span from C-style array. </summary>
    template<size_t N>
    FORCE_INLINE explicit
    Span(T(&array)[N])
        : Span{ array, static_cast<int32>(N) }
    {
    }


    // Properties

    /// <summary> Returns the number of elements in the span. </summary>
    NO_DISCARD FORCE_INLINE
    int32 Count() const
    {
        return _count;
    }

    /// <summary>
    /// Returns the pointer to the underlying data i.e. the first element, unless span length is zero.
    /// </summary>
    /// <remarks>
    /// If the underlying data is <c>null</c>, the span length is also zero.
    /// </remarks>
    NO_DISCARD FORCE_INLINE
    T* Data()
    {
        return _data;
    }

    /// <summary>
    /// Returns the pointer to the underlying data i.e. the first element, unless span length is zero.
    /// </summary>
    /// <remarks>
    /// If the underlying data is <c>null</c>, the span length is also zero.
    /// </remarks>
    NO_DISCARD FORCE_INLINE
    const T* Data() const
    {
        return _data;
    }

    /// <summary> Checks if the span points to any elements. </summary>
    NO_DISCARD FORCE_INLINE explicit
    operator bool() const
    {
        const bool result = _count > 0;
        ASSERT_COLLECTION_INTEGRITY(!result || (_data != nullptr)); // Span data must be valid if the size is non-zero.
        return result;
    }


    // Element Access

    NO_DISCARD FORCE_INLINE
    bool IsValidIndex(const int32 index) const
    {
        const bool obeysRange = (index >= 0 && index < _count); // Index must be within the span range.
        const bool isNotNull = (_data != nullptr); // Span data must be valid if the size is non-zero, implying that the data is not null.
        return obeysRange && isNotNull;
    }

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE
    T& operator[](const int32 index)
    {
        ASSERT_COLLECTION_SAFE_ACCESS(IsValidIndex(index)); // Ensure span is not empty and index is valid
        return _data[index];
    }

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE
    const T& operator[](const int32 index) const
    {
        ASSERT_COLLECTION_SAFE_ACCESS(IsValidIndex(index)); // Ensure span is not empty and index is valid
        return _data[index];
    }


    // Iterators

    /// <summary> Creates a mutable-element enumerator for the span. </summary>
    /// <remarks>
    /// This is the most basic iterator which uses raw memory pointers and does not track the iterated collection.
    /// To be used for any collection storing elements linearly in memory.
    /// </remarks>
    class MutEnumerator
    {
        T* _begin{};
        T* _end{};

    public:
        FORCE_INLINE
        MutEnumerator(T* begin, T* end)
            : _begin{ begin }
            , _end{ end }
        {
            ASSERT_COLLECTION_SAFE_MOD(begin <= end); // Span begin must be before the end.
        }

        FORCE_INLINE
        explicit MutEnumerator(Span& span)
            : _begin{ span.Data() }
            , _end{ span.Data() + span.Count() }
        {
        }


        // Access

        /// <summary> Returns the size hint about the numer of remaining elements. </summary>
        NO_DISCARD FORCE_INLINE
        IterHint Hint() const
        {
            const int32 remaining = static_cast<int32>(_end - _begin);
            return { remaining, remaining };
        }

        
        NO_DISCARD FORCE_INLINE
        T& operator*()
        {
            return *_begin;
        }

        NO_DISCARD FORCE_INLINE
        T* operator->()
        {
            return _begin;
        }

        NO_DISCARD FORCE_INLINE
        const T& operator*() const
        {
            return *_begin;
        }

        NO_DISCARD FORCE_INLINE
        const T* operator->() const
        {
            return _begin;
        }


        // Iteration

        /// <summary> Check if the enumerator points to a valid element. </summary>
        NO_DISCARD FORCE_INLINE explicit
        operator bool() const
        {
            return static_cast<bool>(_begin) && _begin < _end;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        MAY_DISCARD FORCE_INLINE
        MutEnumerator& operator++()
        {
            ++_begin;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        /// <remarks> Prefixed increment operator is faster. </remarks>
        MutEnumerator operator++(int)
        {
            MutEnumerator copy = *this;
            ++(*this);
            return copy;
        }


        // Identity

        NO_DISCARD FORCE_INLINE
        bool operator==(const MutEnumerator& other) const
        {
            return _begin == other._begin && _end == other._end;
        }

        NO_DISCARD FORCE_INLINE
        bool operator!=(const MutEnumerator& other) const
        {
            return _begin != other._begin || _end != other._end;
        }

        NO_DISCARD FORCE_INLINE
        bool operator<(const MutEnumerator& other) const
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_end == other._end); // Enumerators must be of the same span to be compared.
            return _begin < other._begin;
        }
    };


    /// <summary> Creates a const-element enumerator for the span. </summary>
    /// <remarks>
    /// This is the most basic iterator which uses raw memory pointers and does not track the iterated collection.
    /// To be used for any collection storing elements linearly in memory.
    /// </remarks>
    class ConstEnumerator
    {
        const T* _begin{};
        const T* _end{};

    public:
        FORCE_INLINE
        ConstEnumerator(const T* begin, const T* end)
            : _begin{ begin }
            , _end{ end }
        {
            ASSERT_COLLECTION_SAFE_MOD(begin <= end); // Span begin must be before the end.
        }

        FORCE_INLINE explicit
        ConstEnumerator(const Span& span)
            : _begin{ span.Data() }
            , _end{ span.Data() + span.Count() }
        {
        }

        FORCE_INLINE explicit
        ConstEnumerator(const MutEnumerator& enumerator)
            : _begin{ enumerator._begin }
            , _end{ enumerator._end }
        {
        }


        // Access

        NO_DISCARD FORCE_INLINE
        const T& operator*() const
        {
            return *_begin;
        }

        NO_DISCARD FORCE_INLINE
        const T* operator->() const
        {
            return _begin;
        }
        

        // Iteration

        /// <summary> Check if the enumerator points to a valid element. </summary>
        NO_DISCARD FORCE_INLINE explicit
        operator bool() const
        {
            return static_cast<bool>(_begin) && _begin < _end;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        MAY_DISCARD FORCE_INLINE
        ConstEnumerator& operator++()
        {
            ++_begin;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        /// <remarks> Prefixed increment operator is faster. </remarks>
        ConstEnumerator operator++(int)
        {
            ConstEnumerator copy = *this;
            ++(*this);
            return copy;
        }

        
        // Identity

        NO_DISCARD FORCE_INLINE
        bool operator==(const MutEnumerator& other) const
        {
            return _begin == other._begin && _end == other._end;
        }

        NO_DISCARD FORCE_INLINE
        bool operator!=(const MutEnumerator& other) const
        {
            return _begin != other._begin || _end != other._end;
        }

        NO_DISCARD FORCE_INLINE
        bool operator<(const MutEnumerator& other) const
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_end == other._end); // Enumerators must be of the same span to be compared.
            return _begin < other._begin;
        }
    };


    /// <summary> Creates an enumerator for the array. </summary>
    NO_DISCARD FORCE_INLINE
    MutEnumerator Values()
    {
        return MutEnumerator{ *this };
    }

    /// <summary> Creates an enumerator for the array. </summary>
    NO_DISCARD FORCE_INLINE
    ConstEnumerator Values() const
    {
        return ConstEnumerator{ *this };
    }
};
