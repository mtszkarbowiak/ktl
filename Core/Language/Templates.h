// Created by Mateusz Karbowiak 2024

#pragma once

#include <algorithm>


template<typename T>
using VoidT = void;


// Move Semantics

#define MOVE(x)       std::move(x)
#define FORWARD(T, x) std::forward<T>(x)

namespace SwapUtils
{
    struct HasSwapTag {};
    struct NoSwapTag {};

    // Check if T has a member function Swap(T&) noexcept
    template<typename T, typename = void>
    struct HasSwapFunction : std::false_type {};

    // Use SFINAE to check for the Swap function.
    template<typename T>
    struct HasSwapFunction<T, VoidT<decltype(std::declval<T>().Swap(std::declval<T&>()))>> : std::true_type {};

    // Get the appropriate tag based on whether T has a Swap function
    template<typename T>
    using GetSwapTag = std::conditional_t<HasSwapFunction<T>::value, HasSwapTag, NoSwapTag>;

    // SwapImpl for types with a member Swap function
    template<typename T>
    void SwapImpl(T& a, T& b, HasSwapTag) noexcept
    {
        a.Swap(b);
    }

    // SwapImpl for types without a member Swap function
    template<typename T>
    void SwapImpl(T& a, T& b, NoSwapTag) noexcept
    {
        static_assert(
            std::is_nothrow_move_constructible<T>::value,
            "Type must be nothrow move constructible."
        );

        T temp = MOVE(a);
        a = MOVE(b);
        b = MOVE(temp);
    }
}

template<typename T>
void Swap(T& a, T& b) noexcept
{
    ::SwapUtils::SwapImpl(a, b, ::SwapUtils::GetSwapTag<T>{});
}


// Copy Semantics

struct IsCopyableTag {};
struct NonCopyableTag {};

template<typename T>
using GetCopyableTag = std::conditional<
    std::is_copy_constructible<T>::value,
    IsCopyableTag,
    NonCopyableTag
>;
