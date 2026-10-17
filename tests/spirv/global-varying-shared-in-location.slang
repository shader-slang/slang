// Regression test for shader-slang/slang#13687 on SPIR-V.
//
// A module-scope `in` varying with a user-assigned location, read through a shared callee by two
// vertex entry points emitted into one module, must reach each entry point through that entry
// point's own Input variable at that entry point's own location. `vsB` has an extra varying
// parameter, so `gPos` is at location 0 for `vsA` but at location 1 for `vsB`. The shared global
// stays a private variable that each entry point initializes from its own input.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -whole-program
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -whole-program -entry vsB -stage vertex -entry vsA -stage vertex
//TEST:SIMPLE(filecheck=SINGLE): -target spirv-asm -entry vsA -stage vertex

in float3 gPos;

float3 readPos() { return gPos; }

// CHECK-DAG: OpEntryPoint Vertex %vsA "vsA" {{.*}}%[[A_POS:gPos[A-Za-z0-9_]*]]{{$}}
// CHECK-DAG: OpEntryPoint Vertex %vsB "vsB" {{.*}}%extra {{.*}}%[[B_POS:gPos[A-Za-z0-9_]*]]{{$}}
// CHECK-DAG: OpDecorate %[[A_POS]] Location 0
// CHECK-DAG: OpDecorate %[[B_POS]] Location 1
// CHECK-DAG: OpDecorate %extra Location 0

// When only one entry point reaches the global, the global is replaced by the real input.
// SINGLE: OpEntryPoint Vertex %vsA "main" %gl_Position %gPos{{$}}
// SINGLE-NOT: Private

[shader("vertex")]
float4 vsA() : SV_Position
{
    return float4(readPos(), 1.0);
}

[shader("vertex")]
float4 vsB(float4 extra : EXTRA) : SV_Position
{
    return float4(readPos() + extra.xyz, 1.0);
}
