// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include "Language/TypeTraits.h"

#include "Collections/Array.h"

#include <type_traits> // Use standard library type traits for comparison.

// --- Tests for Identity ---
static_assert(std::is_same<TIdentity<int>::Type, int>::value, "TIdentity test failed");

// --- Tests for Flow Control ---
static_assert(std::is_same<TConditionalT<true, int, float>, int>::value,    "TConditional test failed");
static_assert(std::is_same<TConditionalT<false, int, float>, float>::value, "TConditional test failed");
static_assert(std::is_same<TEnableIf<true, int>::Type, int>::value,         "TEnableIf test failed");
static_assert(std::is_same<TDisableIf<false, int>::Type, int>::value,       "TDisableIf test failed");

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
static_assert(std::is_same<TRemoveRef<int&>::Type, int>::value,        "TRemoveRef test failed");
static_assert(std::is_same<TRemovePtr<int*>::Type, int>::value,        "TRemovePointer test failed");
static_assert(std::is_same<TRemoveConst<const int>::Type, int>::value, "TRemoveConst test failed");
static_assert(std::is_same<TAddLValRef<int>::Type, int&>::value,       "TAddLValRef test failed");
static_assert(std::is_same<TAddConst<int>::Type, const int>::value,    "TAddConst test failed");

static_assert(std::is_same<TDecay<int>::Type, int>::value,                "TDecay test failed");
static_assert(std::is_same<TDecay<int&>::Type, int>::value,               "TDecay test failed");
static_assert(std::is_same<TDecay<const int>::Type, int>::value,          "TDecay test failed");
static_assert(std::is_same<TDecay<volatile int>::Type, int>::value,       "TDecay test failed");
static_assert(std::is_same<TDecay<const volatile int>::Type, int>::value, "TDecay test failed");
static_assert(std::is_same<TDecay<int[]>::Type, int*>::value,             "TDecay test failed");
static_assert(std::is_same<TDecay<int[10]>::Type, int*>::value,           "TDecay test failed");
static_assert(std::is_same<TDecay<int(int)>::Type, int(*)(int)>::value,   "TDecay test failed");

// --- Tests for Compiler Intrinsics ---
enum class SomeEnum{ A, B, };
struct SomeStruct { int i; };

static_assert(TIsEnumV<SomeEnum>,   "TIsEnum test failed");
static_assert(!TIsEnumV<int>,       "TIsEnum test failed");
static_assert(TIsPODV<SomeStruct>,  "TIsPOD test failed");
static_assert(!TIsPODV<Array<int>>, "TIsPOD test failed");

static_assert(THasTrivialCtorV<SomeStruct>,  "TIsTriviallyConstructible test failed");
static_assert(THasTrivialCtorV<SomeEnum>,    "TIsTriviallyConstructible test failed");
static_assert(!THasTrivialCtorV<Array<int>>, "TIsTriviallyConstructible test failed");

static_assert(THasTrivialDtorV<SomeStruct>,   "TIsTriviallyDestructible test failed");
static_assert(THasTrivialDtorV<SomeEnum>,     "TIsTriviallyDestructible test failed");
static_assert(!THasTrivialDtorV<Array<int>>,  "TIsTriviallyDestructible test failed");

static_assert(THasTrivialCopyV<SomeStruct>,       "TIsTriviallyCopyable test failed");
static_assert(THasTrivialCopyV<SomeEnum>,         "TIsTriviallyCopyable test failed");
static_assert(!THasTrivialCopyV<Array<int>>,      "TIsTriviallyCopyable test failed");

struct Donkey
{
    Donkey()  = default;
    ~Donkey() = default;

    Donkey(const Donkey&) = delete;
    Donkey(Donkey&&)      = delete;
    auto operator=(const Donkey&) -> Donkey& = delete;
    auto operator=(Donkey&&)      -> Donkey& = delete;
};

static_assert(THasCopyCtorV<SomeStruct>,    "TIsCopyConstructible test failed");
static_assert(!THasCopyCtorV<Donkey>,       "TIsCopyConstructible test failed");
static_assert(THasCopyAssignV<SomeStruct>,  "TIsCopyAssignable test failed");
static_assert(!THasCopyAssignV<Donkey>,     "TIsCopyAssignable test failed");
static_assert(THasMoveCtorV<SomeStruct>,    "TIsMoveConstructible test failed");
static_assert(!THasMoveCtorV<Donkey>,       "TIsMoveConstructible test failed");
static_assert(THasMoveAssignV<SomeStruct>,  "TIsMoveAssignable test failed");
static_assert(!THasMoveAssignV<Donkey>,     "TIsMoveAssignable test failed");
 