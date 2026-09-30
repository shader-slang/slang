// `-layout-rules-version 202c` revises what scalar layout means, so it also applies without
// `-fvk-use-scalar-layout` wherever scalar layout is used: buffers declared with `ScalarDataLayout`,
// and what a user pointer with the default data layout points at. Layouts that already round
// struct sizes (std140 here) are unchanged. `-std 202c` selects the language version only.
//
// `Q.v` is at offset 16 under unrounded scalar layout and at 20 with 202c; std140 or std430 would
// put it at 32, so the offset shows which rules were used. Under std140, `U.w` is at 16 either way.
// Through the pointer `p.pU`, `U.w` is at 12 or 16, and indexing `p.pU[1]` strides by 16 or 24.

//TEST:REFLECTION(filecheck=REFL_OLD):-target spirv -stage compute -entry computeMain
//TEST:REFLECTION(filecheck=REFL_NEW):-target spirv -stage compute -entry computeMain -layout-rules-version 202c
//TEST:REFLECTION(filecheck=REFL_OLD):-target spirv -stage compute -entry computeMain -std 202c
//TEST:SIMPLE(filecheck=SPV_OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPV_OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -std 202c
//TEST:SIMPLE(filecheck=SPV_NEW):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -layout-rules-version 202c

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

struct P
{
    U* pU;
};

ConstantBuffer<Q, ScalarDataLayout> cbScalar;
ConstantBuffer<U> cbDefault;
ConstantBuffer<P> p;
RWStructuredBuffer<Q, ScalarDataLayout> sbScalar;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = cbScalar.v.x;
    output[1] = cbDefault.w;
    output[2] = p.pU[1].w;
    output[3] = sbScalar[0].v.x;
}

// REFL_OLD: "name": "cbScalar"
// REFL_OLD: "name": "w"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 12, "size": 4
// REFL_OLD: "name": "v"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 16, "size": 12
// REFL_OLD: "name": "cbDefault"
// REFL_OLD: "name": "w"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 16, "size": 4
// REFL_OLD: {"kind": "uniform", "value": 32, "alignment": 16}
// REFL_OLD: "name": "p"
// REFL_OLD: "name": "w"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 12, "size": 4

// REFL_NEW: "name": "cbScalar"
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 16, "size": 4
// REFL_NEW: "name": "v"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 20, "size": 12
// REFL_NEW: "name": "cbDefault"
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 16, "size": 4
// REFL_NEW: {"kind": "uniform", "value": 32, "alignment": 16}
// REFL_NEW: "name": "p"
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 16, "size": 4
// REFL_NEW: "name": "sbScalar"
// REFL_NEW: "name": "v"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 20, "size": 12

// SPV_OLD: OpMemberDecorate %Q_natural 1 Offset 12
// SPV_OLD: OpMemberDecorate %Q_natural 2 Offset 16
// SPV_OLD: OpMemberDecorate %U_std140 1 Offset 16
// SPV_OLD: OpDecorate %_ptr_PhysicalStorageBuffer_U_natural ArrayStride 16
// SPV_OLD: OpMemberDecorate %Q_natural_0 2 Offset 16
// SPV_OLD: OpMemberDecorate %U_natural 1 Offset 12

// SPV_NEW: OpMemberDecorate %Q_scalar_rounded 1 Offset 16
// SPV_NEW: OpMemberDecorate %Q_scalar_rounded 2 Offset 20
// SPV_NEW: OpMemberDecorate %U_std140 1 Offset 16
// SPV_NEW: OpDecorate %_ptr_PhysicalStorageBuffer_U_scalar_rounded ArrayStride 24
// SPV_NEW: OpMemberDecorate %Q_scalar_rounded_0 2 Offset 20
// SPV_NEW: OpMemberDecorate %U_scalar_rounded 1 Offset 16
