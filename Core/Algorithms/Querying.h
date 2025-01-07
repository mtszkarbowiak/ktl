// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/Keywords.h"
#include "Language/Templates.h"
#include "Types/Numbers.h"

namespace Querying
{
    // Evaluation Queries

    /// <summary>
    /// Counts the number of elements in the enumerator.
    /// </summary>
    template<typename Enumerator>
    auto static Count(Enumerator&& enumerator) -> int32
    {
        int32 count = 0;
        for (; enumerator; ++enumerator)
            count += 1;
        return count;
    }


    // Transformation Queries

    struct ToCount {}; // Only a tag to indicate the query.


    template<typename Selector>
    struct Select final
    {
        Selector selector;

        Select() = default;

        explicit Select(Selector selector)
            : selector{ selector }
        {
        }
    };

    template<typename Filter>
    struct Where final
    {
        Filter filter;

        Where() = default;

        explicit Where(Filter filter)
            : filter{ filter }
        {
        }
    };

    template<typename Producer, typename Selector>
    class SelectEnumerator final
    {
        Producer _producer; // It must not be a reference, as it would be invalidated.
        Selector _selector;

    public:
        explicit SelectEnumerator(
            Producer&& producer, 
            Selector&& selector)
            : _producer{ MOVE(producer) }
            , _selector{ MOVE(selector) }
        {
        }

        explicit SelectEnumerator(
            Producer&& producer, 
            Select<Selector> selector)
            : _producer{ MOVE(producer) }
            , _selector{ MOVE(selector.selector) }
        {
        }

        FORCE_INLINE
        explicit operator bool() const noexcept
        {
            return static_cast<bool>(_producer);
        }

        FORCE_INLINE
        SelectEnumerator& operator++()
        {
            ++_producer;
            return *this;
        }

        FORCE_INLINE
        SelectEnumerator& operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }

        FORCE_INLINE
        auto operator*()
        {
            return _selector(*_producer);
        }
    };


    template<typename Producer, typename Predicate>
    class WhereEnumerator final
    {
        Producer _producer; // Must not be a reference to avoid invalidation.
        Predicate _predicate;

        // Helper to advance to the next valid element.
        void SkipInvalid()
        {
            while (_producer && !_predicate(*_producer))
            {
                ++_producer;
            }
        }

    public:
        explicit WhereEnumerator(Producer&& producer, Predicate&& predicate)
            : _producer{ MOVE(producer) }
            , _predicate{ MOVE(predicate) }
        {
            SkipInvalid(); // Ensure we start on a valid element.
        }

        explicit WhereEnumerator(Producer&& producer, Where<Predicate> where)
            : _producer{ MOVE(producer) }
            , _predicate{ MOVE(where.filter) }
        {
            SkipInvalid(); // Ensure we start on a valid element.
        }

        FORCE_INLINE
        explicit operator bool() const noexcept
        {
            return static_cast<bool>(_producer); // Valid if the producer is valid.
        }

        FORCE_INLINE
        WhereEnumerator& operator++()
        {
            ++_producer;
            SkipInvalid();
            return *this;
        }

        FORCE_INLINE
        WhereEnumerator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }

        FORCE_INLINE
        decltype(auto) operator*()
        {
            return *_producer; // Return the current element.
        }
    };



    template<typename Producer, typename Selector>
    auto operator|(Producer&& producer, Select<Selector> selector)
    {
        return SelectEnumerator<Producer, Selector>(
            FORWARD(Producer, producer), //TODO This should be r-value, never l-value.
            MOVE(selector)
        );
    }

    template<typename Producer, typename Predicate>
    auto operator|(Producer&& producer, Where<Predicate> where)
    {
        return WhereEnumerator<Producer, Predicate>(
            FORWARD(Producer, producer), //TODO This should be r-value, never l-value.
            MOVE(where)
        );
    }


    template<typename Producer>
    auto operator|(Producer&& producer, ToCount)
    {
        return Count(FORWARD(Producer, producer));
    }

    //TODO Implement First with Array<T> 
    //TODO Implement Last with Ring<T>
    //TODO Implement Skip with int32
    //TODO Implement GroupBy with Dictionary<TKey, Array<TValue>>
    //TODO Implement OrderBy with Array<T>
    //TODO Implement Reverse with Array<T>
    //TODO Implement Distinct with HashSet<T>
}
