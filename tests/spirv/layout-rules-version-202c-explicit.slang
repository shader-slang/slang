// `-layout-rules-version 202c` revises what scalar layout means, so it also applies when scalar
// layout is requested per buffer with `ScalarDataLayout`, without `-fvk-use-scalar-layout`.
// Layouts that already round aggregate sizes (std140 here) are unchanged.
//
// `Q.v` is at offset 16 under unrounded scalar layout and at 20 with 202c; std140 or std430 would
// put it at 32, so the offset shows which rules were used. Under std140, `U.w` is at 16 either way.

//TEST:REFLECTION(filecheck=REFL_OLD):-target spirv -stage compute -entry computeMain
//TEST:REFLECTION(filecheck=REFL_NEW):-target spirv -stage compute -entry computeMain -layout-rules-version 202c
//TEST:SIMPLE(filecheck=SPV_OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain
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

ConstantBuffer<Q, ScalarDataLayout> cbScalar;
ConstantBuffer<U> cbDefault;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = cbScalar.v.x;
    output[1] = cbDefault.w;
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

// REFL_NEW: "name": "cbScalar"
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 16, "size": 4
// REFL_NEW: "name": "v"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 20, "size": 12
// REFL_NEW: "name": "cbDefault"
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 16, "size": 4
// REFL_NEW: {"kind": "uniform", "value": 32, "alignment": 16}

// SPV_OLD: OpMemberDecorate %Q_natural 1 Offset 12
// SPV_OLD: OpMemberDecorate %Q_natural 2 Offset 16
// SPV_OLD: OpMemberDecorate %U_std140 1 Offset 16

// SPV_NEW: OpMemberDecorate %Q_scalar_rounded 1 Offset 16
// SPV_NEW: OpMemberDecorate %Q_scalar_rounded 2 Offset 20
// SPV_NEW: OpMemberDecorate %U_std140 1 Offset 16
