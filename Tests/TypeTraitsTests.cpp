// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include "Language/TypeTraits.h"

#include <type_traits> // Use standard library type traits for comparison.

// --- Tests for Identity ---
static_assert(std::is_same_v<TIdentity<int>::Type, int>, "TIdentity test failed");

// --- Tests for Flow Control ---
static_assert(std::is_same_v<TConditionalT<true, int, float>, int>,    "TConditional test failed");
static_assert(std::is_same_v<TConditionalT<false, int, float>, float>, "TConditional test failed");
static_assert(std::is_same_v<TEnableIf<true, int>::Type, int>,         "TEnableIf test failed");
static_assert(std::is_same_v<TDisableIf<false, int>::Type, int>,       "TDisableIf test failed");

// --- Tests for Logic ---
struct TrueTrait  { static constexpr bool Value = true; };
struct FalseTrait { static constexpr bool Value = false; };
static_assert(TNotV<TrueTrait>             == false, "TNot test failed");
static_assert(TNotV<FalseTrait>            == true,  "TNot test failed");
static_assert(TAndV<TrueTrait, TrueTrait>  == true,  "TAnd test failed");
static_assert(TAndV<TrueTrait, FalseTrait> == false, "TAnd test failed");
static_assert(TOrV<FalseTrait, TrueTrait>  == true,  "TOr test failed");
static_assert(TOrV<FalseTrait, FalseTrait> == false, "TOr test failed");

// --- Tests for Equality ---
static_assert(TIsSameV<int, int>   == true,  "TIsSame test failed");
static_assert(TIsSameV<int, float> == false, "TIsSame test failed");

// --- Tests for Checkers ---
static_assert(TIsLValRefV<int&>    == true,  "TIsLValRef test failed");
static_assert(TIsLValRefV<int>     == false, "TIsLValRef test failed");
static_assert(TIsRValRefV<int&&>   == true,  "TIsRValRef test failed");
static_assert(TIsRefV<int&>        == true,  "TIsRef test failed");
static_assert(TIsRefV<int&&>       == true,  "TIsRef test failed");
static_assert(TIsPtrV<int*>        == true,  "TIsPtr test failed");
static_assert(TIsFuncV<int(int)>   == true,  "TIsFunc test failed");

// --- Tests for Type Modifiers ---
static_assert(std::is_same_v<TRemoveRef<int&>::Type, int>,        "TRemoveRef test failed");
static_assert(std::is_same_v<TRemovePtr<int*>::Type, int>,        "TRemovePointer test failed");
static_assert(std::is_same_v<TRemoveConst<const int>::Type, int>, "TRemoveConst test failed");
static_assert(std::is_same_v<TAddLValRef<int>::Type, int&>,       "TAddLValRef test failed");
static_assert(std::is_same_v<TAddConst<int>::Type, const int>,    "TAddConst test failed");

static_assert(std::is_same_v<TDecay<int>::Type, int>,                "TDecay test failed");
static_assert(std::is_same_v<TDecay<int&>::Type, int>,               "TDecay test failed");
static_assert(std::is_same_v<TDecay<const int>::Type, int>,          "TDecay test failed");
static_assert(std::is_same_v<TDecay<volatile int>::Type, int>,       "TDecay test failed");
static_assert(std::is_same_v<TDecay<const volatile int>::Type, int>, "TDecay test failed");
static_assert(std::is_same_v<TDecay<int[]>::Type, int*>,             "TDecay test failed");
static_assert(std::is_same_v<TDecay<int[10]>::Type, int*>,           "TDecay test failed");
static_assert(std::is_same_v<TDecay<int(int)>::Type, int(*)(int)>,   "TDecay test failed");
