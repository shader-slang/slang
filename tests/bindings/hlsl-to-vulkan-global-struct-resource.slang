// Regression test for shader-slang/slang#14013.
//
// `-fvk-bind-globals 5 9` places the default constant buffer for global uniforms at
// set 9, binding 5. The texture `s.t` is declared through that buffer's element (the
// global uniform `s`), but its binding is allocated in the default set (set 0,
// binding 1), and the SPIR-V must keep that location after legalization splits it out.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry main -stage compute -fvk-bind-globals 5 9
//TEST:REFLECTION(filecheck=REFL): -target spirv -entry main -stage compute -fvk-bind-globals 5 9 -no-codegen

// SPV-DAG: OpDecorate %result Binding 0
// SPV-DAG: OpDecorate %result DescriptorSet 0
// SPV-DAG: OpDecorate %globalParams Binding 5
// SPV-DAG: OpDecorate %globalParams DescriptorSet 9
// SPV-DAG: OpDecorate %globalParams_s_t Binding 1
// SPV-DAG: OpDecorate %globalParams_s_t DescriptorSet 0

// REFL: "name": "s",
// REFL-NEXT: "bindings": [
// REFL-NEXT: {"kind": "descriptorTableSlot", "index": 1},
// REFL: "globalScope": {
// REFL-NEXT: "kind": "constantBuffer",
// REFL-NEXT: "binding": {"kind": "descriptorTableSlot", "space": 9, "index": 5},

RWStructuredBuffer<float> result;

struct S
{
    Texture2D t;
    float f;
}

uniform S s;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    result[0] = s.f + s.t.Load(int3(0)).x;
}
