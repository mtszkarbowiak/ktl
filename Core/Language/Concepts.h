// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/Keywords.h"


#if CONCEPTS_ENABLED
    /// <summary> Tries to use the concept if the compiler supports it. </summary>
    #define TRY_CONCEPT(concept_name) concept_name

#else
    /// <summary> Tries to use the concept if the compiler supports it. </summary>
    #define TRY_CONCEPT(concept_name) typename

#endif
