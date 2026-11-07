//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc-asm -profile ps_5_0 -entry pixelMain -pass-through vkd3d -DVALUE=3 -Itests/downstream/vkd3d/include
//CHECK: ps_5_0
//CHECK: ret

#include "nested/outer.h"

float4 pixelMain() : SV_Target
{
    return float4(getValue(), VALUE, 0, 1);
}
