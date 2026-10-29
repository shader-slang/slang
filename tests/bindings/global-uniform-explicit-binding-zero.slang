// Regression test for shader-slang/slang#14013.
//
// `result` takes binding 0 explicitly, so the default constant buffer that holds the
// global uniforms (`globalParams`) is allocated binding 1. The resources inside the
// global uniform `s` are split out of that buffer during legalization, and they must
// keep the bindings that reflection reports for them: `s.t` at 2 and `u` at 3, and
// reflection reports the default constant buffer itself at 1.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry main -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute -line-directive-mode none
//TEST:REFLECTION(filecheck=REFL): -target spirv -entry main -stage compute -no-codegen

// SPV-DAG: OpDecorate %result Binding 0
// SPV-DAG: OpDecorate %globalParams Binding 1
// SPV-DAG: OpDecorate %globalParams_s_t Binding 2
// SPV-DAG: OpDecorate %u Binding 3

// GLSL-DAG: binding = 0) buffer StructuredBuffer_float_t_0
// GLSL-DAG: layout(binding = 1){{[[:space:]]*}}layout(std140) uniform block_GlobalParams_0
// GLSL-DAG: layout(binding = 2){{[[:space:]]*}}uniform texture2D globalParams_s_t_0;
// GLSL-DAG: layout(binding = 3){{[[:space:]]*}}uniform texture2D u_0;

// WGSL-DAG: @binding(0) @group(0) var<storage, read_write> result_0
// WGSL-DAG: @binding(1) @group(0) var<uniform> globalParams_0
// WGSL-DAG: @binding(2) @group(0) var globalParams_s_t_0
// WGSL-DAG: @binding(3) @group(0) var u_0

// REFL: "name": "s",
// REFL-NEXT: "bindings": [
// REFL-NEXT: {"kind": "descriptorTableSlot", "index": 2},
// REFL: "globalScope": {
// REFL-NEXT: "kind": "constantBuffer",
// REFL-NEXT: "binding": {"kind": "descriptorTableSlot", "index": 1},

[[vk::binding(0)]] RWStructuredBuffer<float> result;

struct S
{
    Texture2D t;
    float f;
}

uniform S s;
Texture2D u;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    result[0] = s.f + s.t.Load(int3(0)).x + u.Load(int3(0)).x;
}
