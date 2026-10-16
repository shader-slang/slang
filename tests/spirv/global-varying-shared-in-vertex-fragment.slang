// Regression test for shader-slang/slang#13687 on SPIR-V.
//
// A module-scope integer `in` varying read through a shared callee by a vertex and a fragment
// entry point emitted into one module must reach each entry point through its own Input
// variable. Only the fragment input of an integer type is decorated `Flat`.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -whole-program
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -whole-program -entry fsB -stage fragment -entry vsA -stage vertex

in uint gId;

uint readId() { return gId; }

// CHECK-DAG: OpEntryPoint Vertex %vsA "vsA" {{.*}}%[[V_ID:gId[A-Za-z0-9_]*]]{{$}}
// CHECK-DAG: OpEntryPoint Fragment %fsB "fsB" {{.*}}%[[F_ID:gId[A-Za-z0-9_]*]]{{$}}
// CHECK-DAG: OpDecorate %[[F_ID]] Flat
// CHECK-NOT: OpDecorate %[[V_ID]] Flat

[shader("vertex")]
float4 vsA() : SV_Position
{
    return float4(readId(), 0, 0, 1);
}

[shader("fragment")]
float4 fsB() : SV_Target
{
    return float4(readId(), 0, 0, 1);
}
