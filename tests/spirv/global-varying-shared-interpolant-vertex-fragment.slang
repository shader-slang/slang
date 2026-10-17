// Regression test for shader-slang/slang#13687 on SPIR-V.
//
// A vertex entry point reads a module-scope input directly and a fragment entry point passes the
// same input to `interpolateAtCentroid`. Both place it at location 0, so they share one `Input`
// variable even though their stages differ.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -whole-program
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -whole-program -entry fsB -stage fragment -entry vsA -stage vertex

import glsl;

in float4 vColor;

// CHECK-DAG: OpEntryPoint Vertex %vsA "vsA" {{.*}}%vColor{{ |$}}
// CHECK-DAG: OpEntryPoint Fragment %fsB "fsB" {{.*}}%vColor{{ |$}}
// CHECK-DAG: OpDecorate %vColor Location 0
// CHECK-DAG: InterpolateAtCentroid %vColor

[shader("vertex")]
float4 vsA() : SV_Position
{
    return vColor;
}

[shader("fragment")]
float4 fsB() : SV_Target
{
    return interpolateAtCentroid(vColor);
}
