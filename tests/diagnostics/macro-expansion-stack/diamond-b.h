// File B: includes A and expands ERRORING_MACRO
#pragma once
#include "diamond-a.h"

void testB()
{
    ERRORING_MACRO
}
