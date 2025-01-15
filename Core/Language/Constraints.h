// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include <type_traits>


#define REQUIRE_TYPE_NOT_CONST(T) \
    static_assert(!std::is_const<T>::value, "Type must not be const.")

#define REQUIRE_TYPE_NOT_REFERENCE(T) \
    static_assert(!std::is_reference<T>::value, "Type must not be a reference type.")

#define REQUIRE_TYPE_NOT_VOID(T) \
    static_assert(!std::is_void<T>::value, "Type must not be void.")

#define REQUIRE_TYPE_NOT_ARRAY(T) \
    static_assert(!std::is_array<T>::value, "Type must not be an array.")

#define REQUIRE_TYPE_MOVEABLE_NOEXCEPT(T) \
    static_assert( \
        std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value, \
        "Type must be nothrow move constructible and assignable.")
