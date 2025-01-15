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
#include "Types/IterHint.h"

namespace Querying
{
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
    class SelectCursor final
    {
        C _cursor;
        P _projection;

    public:
        using ElementType = decltype(_projection.operator()(*_cursor));
        
        FORCE_INLINE explicit
        SelectCursor(
            C&& cursor,
            P&& projection)
            : _cursor{ MOVE(cursor) }
            , _projection{ MOVE(projection) }
        {
        }

        FORCE_INLINE explicit
        SelectCursor(
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
        auto operator++() -> SelectCursor&
        {
            ++_cursor;
            return *this;
        }

        MAY_DISCARD FORCE_INLINE
        auto operator++(int) -> SelectCursor&
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
            // SelectCursor does not change the number of elements.
            return _cursor.Hint();
        }
    };

    /// <summary> Maps the elements of the collection using the specified projection. </summary>
    /// <typeparam name="C"> Type of the cursor pointing to the elements. </typeparam>
    /// <typeparam name="P"> Type of the projection function. </typeparam>
    template<typename _C, typename _P>
    NO_DISCARD FORCE_INLINE 
    auto operator|(_C&& cursor, Select<_P> tag) -> SelectCursor<_C, _P>
    {
        return SelectCursor<_C, _P>(
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
    class WhereCursor final
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
        WhereCursor(C&& cursor, P&& predicate)
            : _cursor{ MOVE(cursor) }
            , _predicate{ MOVE(predicate) }
        {
            SkipInvalid(); // Ensure we start on a valid element.
        }

        FORCE_INLINE explicit
        WhereCursor(C&& cursor, Where<P> where)
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
        auto operator++() -> WhereCursor&
        {
            ++_cursor;
            SkipInvalid();
            return *this;
        }

        MAY_DISCARD FORCE_INLINE
        auto operator++(int) -> WhereCursor
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
            // WhereCursor may reduce the number of elements.
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
    auto operator|(_C&& cursor, Where<P> where) -> WhereCursor<_C, P>
    {
        return WhereCursor<_C, P>(
            FORWARD(_C, cursor), //TODO This should be r-value, never l-value.
            MOVE(where)
        );
    }


    // Evaluation

    /// <summary> Tag used to indicate that the query should count the elements. </summary>
    struct ToCount
    {
        // Empty
    };

    /// <summary> Counts the number of elements in the collection. </summary>
    /// <typeparam name="C"> Type of the cursor pointing to the elements. </typeparam>
    template<typename _C> // Universal reference
    NO_DISCARD FORCE_INLINE
    auto static Count(_C&& cursor) -> int32
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


    //TODO(mtszkarbowiak) Implement First with Array<T> 
    //TODO(mtszkarbowiak) Implement Last with Ring<T>
    //TODO(mtszkarbowiak) Implement Skip with int32
}
