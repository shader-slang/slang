// Regression test for shader-slang/slang#13687 on SPIR-V.
//
// A module-scope struct-typed `in` varying read through a shared callee by two fragment entry
// points emitted into one module must give each entry point its own Input variables, so that
// no entry point's interface lists two variables at the same location.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -whole-program
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -whole-program -entry fsB -stage fragment -entry fsA -stage fragment

struct FIn
{
    float4 c : COLOR0;
    float2 uv : TEXCOORD0;
};
in FIn gIn;

float4 readIn() { return gIn.c + float4(gIn.uv, 0, 0); }

// CHECK-DAG: OpEntryPoint Fragment %fsA "fsA" {{.*}}%[[A_C:gIn_c[A-Za-z0-9_]*]] %[[A_UV:gIn_uv[A-Za-z0-9_]*]]{{$}}
// CHECK-DAG: OpEntryPoint Fragment %fsB "fsB" {{.*}}%[[B_C:gIn_c[A-Za-z0-9_]*]] %[[B_UV:gIn_uv[A-Za-z0-9_]*]]{{$}}
// CHECK-DAG: OpDecorate %[[A_C]] Location 0
// CHECK-DAG: OpDecorate %[[A_UV]] Location 1
// CHECK-DAG: OpDecorate %[[B_C]] Location 0
// CHECK-DAG: OpDecorate %[[B_UV]] Location 1

[shader("fragment")]
float4 fsA() : SV_Target
{
    return readIn();
}

[shader("fragment")]
float4 fsB() : SV_Target
{
    return readIn() * 2.0;
}
