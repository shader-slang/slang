//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc-asm -profile ps_5_0 -entry pixelMain -pass-through vkd3d -DIDENTITY(x)=x -DSCALE(x,y)=((x)*(y)) -DBASE=3
//CHECK: ps_5_0
//CHECK: l(6.00000000e+00
//CHECK: ret

#ifndef IDENTITY
#error Missing function macro
#endif

#if SCALE(IDENTITY(BASE), 2) != 6
#error Incorrect function macro expansion
#endif

float4 pixelMain() : SV_Target
{
    return float4(SCALE(IDENTITY(BASE), 2), 0, 0, 1);
}
