// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"
#include "Language/Templates.h"
#include "Types/Numbers.h"

namespace Querying
{
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

    template<typename Selector>
    struct Select
    {
        Selector selector;

        Select() = default;

        explicit Select(Selector selector)
            : selector{ selector }
        {
        }
    };

    template<typename Producer, typename Selector>
    class SelectEnumerator
    {
        Producer&& _producer;
        Selector   _selector;

    public:
        explicit SelectEnumerator(
            Producer&& producer, 
            Selector selector)
            : _producer{ FORWARD(Producer, producer) }
            , _selector{ selector }
        {
        }

        explicit SelectEnumerator(
            Producer&& producer, 
            Select<Selector> selector)
            : _producer{ FORWARD(Producer, producer) }
            , _selector{ selector.selector }
        {
        }


        using InputElement  = decltype(*_producer);
        using OutputElement = decltype(_selector(*_producer));

        FORCE_INLINE NODISCARD
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

        FORCE_INLINE NODISCARD
        auto operator*()
        {
            return _selector(*_producer);
        }
    };


    template<typename Producer, typename Selector>
    auto operator|(Producer&& producer, Select<Selector> selector)
    {
        return SelectEnumerator<Producer, Selector>(
            FORWARD(Producer, producer),
            selector
        );
    }
};
