// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/Keywords.h"
#include "Language/Templates.h"
#include "Types/Numbers.h"
#include "Types/SizeHint.h"

namespace Querying
{
    // Utility Pullers

    // Mapping

    /// <summary> Tag used to indicate that the query should map the elements. </summary>
    /// <typeparam name="P"> Type of the projection function. </typeparam>
    template<typename P>
    struct Select final
    {
        P Projection{};

        FORCE_INLINE
        Select() = default;

        FORCE_INLINE explicit
        Select(P projection) // Reference?
            : Projection{ projection }
        {
        }
    };

    /// <summary> Maps the elements of the collection using the specified projection. </summary>
    /// <typeparam name="C"> Type of the cursor pointing to the elements. </typeparam>
    /// <typeparam name="P"> Type of the projection function. </typeparam>
    template<typename C, typename P>
    class SelectPuller final
    {
        C _cursor;
        P _projection;

    public:
        using ElementType = decltype(_projection.operator()(*_cursor));
        
        FORCE_INLINE explicit
        SelectPuller(
            C&& cursor,
            P&& projection)
            : _cursor{ MOVE(cursor) }
            , _projection{ MOVE(projection) }
        {
        }

        FORCE_INLINE explicit
        SelectPuller(
            C&& producer,
            Select<P> select)
            : _cursor{ MOVE(producer) }
            , _projection{ MOVE(select.Projection) }
        {
        }

        NO_DISCARD FORCE_INLINE explicit
        operator bool() const
        {
            return static_cast<bool>(_cursor);
        }

        MAY_DISCARD FORCE_INLINE
        auto operator++() -> SelectPuller&
        {
            ++_cursor;
            return *this;
        }

        MAY_DISCARD FORCE_INLINE
        auto operator++(int) -> SelectPuller&
        {
            auto copy = *this;
            ++*this;
            return copy;
        }

        NO_DISCARD FORCE_INLINE
        auto operator*() -> ElementType
        {
            return _projection.operator()(*_cursor);
        }


        NO_DISCARD FORCE_INLINE
        auto Hint() const -> SizeHint
        {
            // SelectPuller does not change the number of elements.
            return _cursor.Hint();
        }
    };

    /// <summary> Maps the elements of the collection using the specified projection. </summary>
    /// <typeparam name="C"> Type of the cursor pointing to the elements. </typeparam>
    /// <typeparam name="P"> Type of the projection function. </typeparam>
    template<typename _C, typename _P>
    NO_DISCARD FORCE_INLINE 
    auto operator|(_C&& cursor, Select<_P>&& tag) -> SelectPuller<_C, _P>
    {
        return SelectPuller<_C, _P>(
            FORWARD(_C, cursor),
            MOVE(tag)
        );
    }



    // Filtering

    /// <summary> Tag used to indicate that the query should filter the elements. </summary>
    /// <typeparam name="P"> Type of the predicate function. </typeparam>
    template<typename P>
    struct Where final
    {
        P Predicate{};

        FORCE_INLINE
        Where() = default;

        FORCE_INLINE explicit
        Where(P predicate)
            : Predicate{ predicate }
        {
        }
    };

    /// <summary> Filters the elements of the collection using the specified predicate. </summary>
    /// <typeparam name="C"> Type of the cursor pointing to the elements. </typeparam>
    /// <typeparam name="P"> Type of the predicate function. </typeparam>
    template<typename C, typename P>
    class WherePuller final
    {
        C _cursor;
        P _predicate;

        // Helper to advance to the next valid element.
        FORCE_INLINE
        void SkipInvalid()
        {
            while (_cursor && !_predicate(*_cursor))
            {
                ++_cursor;
            }
        }

    public:
        using ElementType = decltype(*_cursor);

        FORCE_INLINE explicit
        WherePuller(C&& cursor, P&& predicate)
            : _cursor{ MOVE(cursor) }
            , _predicate{ MOVE(predicate) }
        {
            SkipInvalid(); // Ensure we start on a valid element.
        }

        FORCE_INLINE explicit
        WherePuller(C&& cursor, Where<P> where)
            : _cursor{ MOVE(cursor) }
            , _predicate{ MOVE(where.Predicate) }
        {
            SkipInvalid(); // Ensure we start on a valid element.
        }

        NO_DISCARD FORCE_INLINE explicit
        operator bool() const
        {
            return static_cast<bool>(_cursor); // Valid if the producer is valid.
        }

        MAY_DISCARD FORCE_INLINE
        auto operator++() -> WherePuller&
        {
            ++_cursor;
            SkipInvalid();
            return *this;
        }

        MAY_DISCARD FORCE_INLINE
        auto operator++(int) -> WherePuller
        {
            auto copy = *this;
            ++*this;
            return copy;
        }

        NO_DISCARD FORCE_INLINE
        auto operator*() -> ElementType
        {
            return *_cursor;
        }

        NO_DISCARD FORCE_INLINE
        auto Hint() const -> SizeHint
        {
            // WherePuller may reduce the number of elements.
            // Yet currently, there is no way to know how many elements will be skipped.
            // In the future an advanced hint system could be implemented.
            return _cursor.Hint();
        }
    };

    /// <summary> Filters the elements of the collection using the specified predicate. </summary>
    /// <typeparam name="_C"> Type of the cursor pointing to the elements. </typeparam>
    /// <typeparam name="P"> Type of the predicate function. </typeparam>
    template<typename _C, typename P>
    NO_DISCARD FORCE_INLINE
    auto operator|(_C&& cursor, Where<P>&& where) -> WherePuller<_C, P>
    {
        return WherePuller<_C, P>(
            FORWARD(_C, cursor),
            MOVE(where)
        );
    }



    // Evaluation

    // Count

    /// <summary> Tag used to indicate that the query should count the elements. </summary>
    struct ToCount
    {
        // Empty
    };

    /// <summary> Counts the number of elements in the collection. </summary>
    /// <typeparam name="C"> Type of the cursor pointing to the elements. </typeparam>
    template<typename _C> // Universal reference
    NO_DISCARD FORCE_INLINE
    auto Count(_C&& cursor) -> int32
    {
        int32 count = 0;
        for (; cursor; ++cursor)
            count += 1;
        return count;
    }

    /// <summary> Counts the number of elements in the collection. </summary>
    /// <typeparam name="C"> Type of the cursor pointing to the elements. </typeparam>
    template<typename _C>
    NO_DISCARD FORCE_INLINE
    auto operator|(_C&& cursor, ToCount) -> int32
    {
        return Count(FORWARD(_C, cursor));
    }


    // Containment (by pointer)

    /// <summary> Tag used to indicate that the query should check if the collection contains the specified value. </summary>
    template<typename T>
    struct ToContainsPtr final
    {
        const T* Value;

        FORCE_INLINE explicit
        ToContainsPtr(const T& value)
            : Value{ &value }
        {
        }
    };

    /// <summary> Checks if the collection contains the specified value. </summary>
    template<typename _C, typename T>
    NO_DISCARD FORCE_INLINE
    auto ContainsPtr(_C&& cursor, const T& value) -> bool
    {
        for (; cursor; ++cursor)
        {
            if (*cursor == value)
                return true;
        }
        return false;
    }

    /// <summary> Checks if the collection contains the specified value. </summary>
    template<typename _C, typename T>
    NO_DISCARD FORCE_INLINE
    auto operator|(_C&& cursor, ToContainsPtr<T>&& contains) -> bool
    {
        return ContainsPtr(FORWARD(_C, cursor), *contains.Value);
    }


    // Containment (by value)

    /// <summary> Tag used to indicate that the query should check if the collection contains the specified value. </summary>
    template<typename T>
    struct ToContains final
    {
        T Value;

        FORCE_INLINE explicit
        ToContains(T&& value)
            : Value{ MOVE(value) }
        {
        }
    };

    /// <summary> Checks if the collection contains the specified value. </summary>
    template<typename _C, typename T>
    NO_DISCARD FORCE_INLINE
    auto Contains(_C&& cursor, T&& value) -> bool
    {
        for (; cursor; ++cursor)
        {
            if (*cursor == value)
                return true;
        }
        return false;
    }

    /// <summary> Checks if the collection contains the specified value. </summary>
    template<typename _C, typename T>
    NO_DISCARD FORCE_INLINE
    auto operator|(_C&& cursor, ToContains<T>&& contains) -> bool
    {
        return Contains(FORWARD(_C, cursor), MOVE(contains.Value));
    }


    // Any

    /// <summary> Tag used to indicate that the query should check if any element satisfies the predicate. </summary>
    template<typename P>
    struct ToAny final
    {
        P Predicate;

        FORCE_INLINE explicit
        ToAny(P&& predicate)
            : Predicate{ MOVE(predicate) }
        {
        }
    };

    /// <summary> Checks if any element satisfies the predicate. </summary>
    template<typename _C, typename P>
    NO_DISCARD FORCE_INLINE
    auto Any(_C&& cursor, P&& predicate) -> bool
    {
        for (; cursor; ++cursor)
        {
            const bool result = predicate(*cursor);
            if (result)
                return true;
        }
        return false;
    }

    /// <summary> Checks if any element satisfies the predicate. </summary>
    template<typename _C, typename P>
    auto operator|(_C&& cursor, ToAny<P>&& any) -> bool
    {
        return Any(FORWARD(_C, cursor), MOVE(any.Predicate));
    }


    // All

    /// <summary> Tag used to indicate that the query should check if all elements satisfy the predicate. </summary>
    template<typename P>
    struct ToAll final
    {
        P Predicate;

        FORCE_INLINE explicit
        ToAll(P&& predicate)
            : Predicate{ MOVE(predicate) }
        {
        }
    };

    /// <summary> Checks if all elements satisfy the predicate. </summary>
    template<typename _C, typename P>
    NO_DISCARD FORCE_INLINE
    auto All(_C&& cursor, P&& predicate) -> bool
    {
        for (; cursor; ++cursor)
        {
            const bool result = predicate(*cursor);
            if (!result)
                return false;
        }
        return true;
    }

    /// <summary> Checks if all elements satisfy the predicate. </summary>
    template<typename _C, typename P>
    NO_DISCARD FORCE_INLINE
    auto operator|(_C&& cursor, ToAll<P>&& all) -> bool
    {
        return All(FORWARD(_C, cursor), MOVE(all.Predicate));
    }


    // First

    /// <summary> Tag used to indicate that the query should return a pointer to the first element. </summary>
    struct ToFirst final
    {
        // Empty
    };

    /// <summary> Returns a pointer to the first element. </summary>
    template<typename _C>
    NO_DISCARD FORCE_INLINE
    auto First(_C&& cursor) -> decltype(&*cursor)
    {
        return (cursor) ? &*cursor : nullptr;
    }

    /// <summary> Returns a pointer to the first element. </summary>
    template<typename _C>
    NO_DISCARD FORCE_INLINE
    auto operator|(_C&& cursor, ToFirst) -> decltype(&*cursor)
    {
        return First(FORWARD(_C, cursor));
    }


    // Last

    /// <summary> Tag used to indicate that the query should return a pointer to the last element. </summary>
    struct ToLast final
    {
        // Empty
    };

    /// <summary> Returns a pointer to the last element. </summary>
    template<typename _C>
    NO_DISCARD FORCE_INLINE
    auto Last(_C&& cursor) -> decltype(&*cursor)
    {
        using ElementType = TRemoveRefT<decltype(*cursor)>;

        ElementType* last = nullptr;
        for (; cursor; ++cursor)
            last = &*cursor;
        return last;
    }

    /// <summary> Returns a pointer to the last element. </summary>
    template<typename _C>
    NO_DISCARD FORCE_INLINE
    auto operator|(_C&& cursor, ToLast) -> decltype(&*cursor)
    {
        return Last(FORWARD(_C, cursor));
    }
}
