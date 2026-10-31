//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc-asm -profile ps_5_0 -entry pixelMain -pass-through vkd3d -Itests/downstream/vkd3d/include
//CHECK: ps_5_0
//CHECK: ret

#include <include-choice.h>
#if INCLUDE_CHOICE != 2
#error Wrong system include search order
#endif

#undef INCLUDE_CHOICE
#include "include-choice.h"
#if INCLUDE_CHOICE != 1
#error Wrong local include search order
#endif

#include <nested/system-outer.h>

float4 pixelMain() : SV_Target
{
    return float4(INCLUDE_CHOICE, 0, 0, 1);
}
