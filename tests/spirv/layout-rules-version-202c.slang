// `-layout-rules-version 202c` rounds the size of every aggregate up to a multiple of its alignment
// under scalar layout, matching DXC. Reflection and the emitted SPIR-V must agree on the result, so
// each configuration below is checked both ways from the same source.
//
// `S` has 4 bytes of tail padding under C rules: 12 bytes of data with 8-byte alignment. Unrounded
// scalar layout keeps `S` at 12 bytes, so `U.w` lands at offset 12 and `U` is 16 bytes. With 202c
// `S` becomes 16 bytes, which moves `U.w` to offset 16 and makes `U` 24 bytes.
//
// `Q` adds a `float3` after `w`, which separates rounded scalar layout from std430: `Q.v` is at 16
// (unrounded scalar), 20 (rounded scalar), or 32 (std430). The pointee of a default user pointer uses
// scalar layout, so `Q*` checks that pointers follow the layout rules version too.
//
// `latest` names 2026, so it keeps the old rules; `next` names 202c.

//TEST:REFLECTION(filecheck=REFL_OLD):-target spirv -stage compute -entry computeMain -fvk-use-scalar-layout
//TEST:REFLECTION(filecheck=REFL_OLD):-target spirv -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 2026
//TEST:REFLECTION(filecheck=REFL_NEW):-target spirv -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c
//TEST:SIMPLE(filecheck=SPV_OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout
//TEST:SIMPLE(filecheck=SPV_OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 2026
//TEST:SIMPLE(filecheck=SPV_NEW):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c
//TEST:SIMPLE(filecheck=SPV_OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version latest
//TEST:SIMPLE(filecheck=SPV_NEW):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version next

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

struct Q
{
    S z;
    float w;
    float3 v;
};

struct Params
{
    Q* pQ;
};

ConstantBuffer<U> cb;
RWStructuredBuffer<U> sb;
ConstantBuffer<Q> cbQ;
ConstantBuffer<Params> params;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    output[0] = cb.w;
    output[1] = sb[tid.x].w;
    output[2] = cbQ.v.x;
    output[3] = params.pQ->v.y;
}

// Reflection, unset or 2026: `S` is 12 bytes, `U.w` is at 12, and `U` is 16 bytes, for both the
// constant buffer and the structured buffer element. The pointee `Q.v` is at 16.
// REFL_OLD: "name": "cb"
// REFL_OLD: "name": "S"
// REFL_OLD: {"kind": "uniform", "value": 12, "alignment": 8}
// REFL_OLD: "name": "w"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 12, "size": 4
// REFL_OLD: {"kind": "uniform", "value": 16, "alignment": 8}
// REFL_OLD: "name": "sb"
// REFL_OLD: "name": "w"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 12, "size": 4
// REFL_OLD: {"kind": "uniform", "value": 16, "alignment": 8}
// REFL_OLD: "name": "cbQ"
// REFL_OLD: "name": "v"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 16, "size": 12
// REFL_OLD: "name": "params"
// REFL_OLD: "name": "v"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 16, "size": 12

// Reflection, 202c: `S` rounds to 16 bytes, `U.w` moves to 16, `U` is 24 bytes, and `Q.v` is at 20.
// REFL_NEW: "name": "cb"
// REFL_NEW: "name": "S"
// REFL_NEW: {"kind": "uniform", "value": 16, "alignment": 8}
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 16, "size": 4
// REFL_NEW: {"kind": "uniform", "value": 24, "alignment": 8}
// REFL_NEW: "name": "sb"
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 16, "size": 4
// REFL_NEW: {"kind": "uniform", "value": 24, "alignment": 8}
// REFL_NEW: "name": "cbQ"
// REFL_NEW: "name": "v"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 20, "size": 12
// REFL_NEW: "name": "params"
// REFL_NEW: "name": "v"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 20, "size": 12

// Emitted SPIR-V uses the same offsets and strides as reflection. Each lowered struct is named
// after its layout rule: `cb` points at `U_*`, the `sb` element is `U_*_0`, `cbQ` points at `Q_*`,
// and the pointer in `params` points at `Q_*_0`. `U`'s rounded numbers happen to coincide with
// std430, so the `Q.v` offsets (16 unrounded, 20 rounded, 32 under std430) are what show that the
// constant buffer and the pointee use rounded scalar layout.
// SPV_OLD: OpMemberDecorate %U_natural 1 Offset 12
// SPV_OLD: OpMemberDecorate %U_natural_0 1 Offset 12
// SPV_OLD: OpDecorate %_runtimearr_U_natural_0 ArrayStride 16
// SPV_OLD: OpMemberDecorate %Q_natural 2 Offset 16
// SPV_OLD: OpMemberDecorate %Q_natural_0 2 Offset 16
// SPV_OLD: %_ptr_PhysicalStorageBuffer_Q_natural_0 = OpTypePointer PhysicalStorageBuffer %Q_natural_0
// SPV_OLD: %cb = OpVariable %_ptr_Uniform_U_natural Uniform
// SPV_OLD: %cbQ = OpVariable %_ptr_Uniform_Q_natural Uniform

// SPV_NEW: OpMemberDecorate %U_scalar_rounded 1 Offset 16
// SPV_NEW: OpMemberDecorate %U_scalar_rounded_0 1 Offset 16
// SPV_NEW: OpDecorate %_runtimearr_U_scalar_rounded_0 ArrayStride 24
// SPV_NEW: OpMemberDecorate %Q_scalar_rounded 2 Offset 20
// SPV_NEW: OpMemberDecorate %Q_scalar_rounded_0 2 Offset 20
// SPV_NEW: %_ptr_PhysicalStorageBuffer_Q_scalar_rounded_0 = OpTypePointer PhysicalStorageBuffer %Q_scalar_rounded_0
// SPV_NEW: %cb = OpVariable %_ptr_Uniform_U_scalar_rounded Uniform
// SPV_NEW: %cbQ = OpVariable %_ptr_Uniform_Q_scalar_rounded Uniform
