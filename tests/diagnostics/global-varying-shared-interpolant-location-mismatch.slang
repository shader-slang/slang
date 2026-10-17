// Regression test for shader-slang/slang#13687: a module-scope input passed to an interpolation
// function is shared as one input variable by the entry points that reach it, which is only
// valid if they place it at the same location. `fsB` has its own varying parameter, so `vColor`
// would follow it at location 1 in `fsB` but sit at location 0 in `fsA`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv-asm -whole-program

import glsl;

in float4 vColor;
//CHECK:  ^^^^^^ error E58004
//CHECK:  ^^^^^^ 'fsB' would place it at a different location or stage than 'fsA'

float4 readColor() { return interpolateAtCentroid(vColor); }

[shader("fragment")]
float4 fsA() : SV_Target
{
    return readColor();
}

[shader("fragment")]
float4 fsB(float4 extra : EXTRA) : SV_Target
{
    return readColor() * extra;
}
