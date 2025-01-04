// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"


#if CONCEPTS_ENABLED
    /// <summary> Tries to use the concept if the compiler supports it. </summary>
    #define TRY_CONCEPT(concept_name) concept_name

#else
    /// <summary> Tries to use the concept if the compiler supports it. </summary>
    #define TRY_CONCEPT(concept_name) typename

#endif
