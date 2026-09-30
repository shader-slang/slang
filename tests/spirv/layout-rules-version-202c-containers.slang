// `-layout-rules-version 202c` applies to parameter blocks and GLSL-style shader storage buffers as
// well as to the constant and structured buffers of layout-rules-version-202c.slang, and to arrays
// of structs. Each case is checked in
// reflection and in the emitted SPIR-V. `-std 202c` selects the language version only, so it
// leaves layout unchanged.
//
// `U.w` is at offset 12 unrounded and 16 rounded. In `V`, the array `S arr[2]` has a 16-byte
// stride either way (8-byte alignment), but its size is 28 unrounded (16 + 12) and 32 rounded, so
// `V.w` moves from 28 to 32.

//TEST:REFLECTION(filecheck=REFL_OLD):-target spirv -stage compute -entry computeMain -fvk-use-scalar-layout
//TEST:REFLECTION(filecheck=REFL_OLD):-target spirv -stage compute -entry computeMain -fvk-use-scalar-layout -std 202c
//TEST:REFLECTION(filecheck=REFL_NEW):-target spirv -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c
//TEST:SIMPLE(filecheck=SPV_OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout
//TEST:SIMPLE(filecheck=SPV_OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout -std 202c
//TEST:SIMPLE(filecheck=SPV_NEW):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c

struct S
{
    double x;
    int y;
};

struct U
{
    S z;
    float w;
};

struct V
{
    S arr[2];
    float w;
};

ParameterBlock<U> pb;
ConstantBuffer<V> cbV;
GLSLShaderStorageBuffer<U> ssbo;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = pb.w;
    output[1] = cbV.w;
    output[2] = ssbo.w;
}

// REFL_OLD: "name": "pb"
// REFL_OLD: "name": "w"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 12, "size": 4
// REFL_OLD: {"kind": "uniform", "value": 16, "alignment": 8}
// REFL_OLD: "name": "cbV"
// REFL_OLD: "name": "arr"
// REFL_OLD: "uniformStride": 16
// REFL_OLD: "name": "w"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 28, "size": 4
// REFL_OLD: "name": "ssbo"
// REFL_OLD: "name": "w"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 12, "size": 4

// REFL_NEW: "name": "pb"
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 16, "size": 4
// REFL_NEW: {"kind": "uniform", "value": 24, "alignment": 8}
// REFL_NEW: "name": "cbV"
// REFL_NEW: "name": "arr"
// REFL_NEW: "uniformStride": 16
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 32, "size": 4
// REFL_NEW: "name": "ssbo"
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 16, "size": 4

// SPV_OLD: OpMemberDecorate %U_natural 1 Offset 12
// SPV_OLD: OpDecorate %_arr_S_natural_int_2 ArrayStride 16
// SPV_OLD: OpMemberDecorate %V_natural 1 Offset 28
// SPV_OLD: OpMemberDecorate %U_natural_0 1 Offset 12
// SPV_OLD: %ssbo = OpVariable %_ptr_StorageBuffer_U_natural_0 StorageBuffer

// SPV_NEW: OpMemberDecorate %U_scalar_rounded 1 Offset 16
// SPV_NEW: OpDecorate %_arr_S_scalar_rounded_int_2 ArrayStride 16
// SPV_NEW: OpMemberDecorate %V_scalar_rounded 1 Offset 32
// SPV_NEW: OpMemberDecorate %U_scalar_rounded_0 1 Offset 16
// SPV_NEW: %ssbo = OpVariable %_ptr_StorageBuffer_U_scalar_rounded_0 StorageBuffer
