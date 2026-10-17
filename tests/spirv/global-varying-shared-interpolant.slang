// Regression test for shader-slang/slang#13687 on SPIR-V.
//
// `interpolateAtCentroid` reads its argument through the address of a real input variable, so a
// module-scope input passed to it from a callee shared by two fragment entry points must stay
// one `Input` variable that both entry points list in their interfaces, not a private copy.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -whole-program
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -whole-program -entry fsB -stage fragment -entry fsA -stage fragment

import glsl;

in float4 vColor;

float4 readColor() { return interpolateAtCentroid(vColor); }

// CHECK-DAG: OpEntryPoint Fragment %fsA "fsA" {{.*}}%vColor{{ |$}}
// CHECK-DAG: OpEntryPoint Fragment %fsB "fsB" {{.*}}%vColor{{ |$}}
// CHECK-DAG: OpDecorate %vColor Location 0
// CHECK-DAG: %vColor = OpVariable %{{[A-Za-z0-9_]+}} Input
// CHECK-DAG: InterpolateAtCentroid %vColor

[shader("fragment")]
float4 fsA() : SV_Target
{
    return readColor();
}

[shader("fragment")]
float4 fsB() : SV_Target
{
    return readColor() * 2.0;
}
