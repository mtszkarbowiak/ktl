// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/TypeTraits.h"

#define REQUIRE_TYPE_NOT_CONST(T) \
    static_assert(!TIsConstV<T>, "Type must not be const.")

#define REQUIRE_TYPE_NOT_REFERENCE(T) \
    static_assert(!TIsRefV<T>, "Type must not be a reference type.")

#define REQUIRE_TYPE_NOT_VOID(T) \
    static_assert(!TIsVoidV<T>, "Type must not be void.")

#define REQUIRE_TYPE_NOT_ARRAY(T) \
    static_assert(!TIsArrayV<T>, "Type must not be an array.")

#define REQUIRE_TYPE_MOVEABLE(T) \
    static_assert(TIsMoveableV<T>, "Type must be moveable.")
