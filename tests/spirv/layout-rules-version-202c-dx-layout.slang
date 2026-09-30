// Under `-fvk-use-dx-layout`, structured buffers use natural layout, which is not scalar layout and
// so is not rounded by `-layout-rules-version 202c`, in reflection or in the emitted SPIR-V. What a pointer points at is still scalar
// layout, as is a buffer declared with `ScalarDataLayout`, and both follow 202c.
//
// `U.w` is at offset 12 in a 16-byte `U` without rounding, and at 16 in a 24-byte `U` with it.
// `Q.v` is at 16 without rounding and at 20 with it.

//TEST:REFLECTION(filecheck=REFL_OLD):-target spirv -stage compute -entry computeMain -fvk-use-dx-layout
//TEST:REFLECTION(filecheck=REFL_NEW):-target spirv -stage compute -entry computeMain -fvk-use-dx-layout -layout-rules-version 202c
//TEST:SIMPLE(filecheck=SPV_OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-dx-layout
//TEST:SIMPLE(filecheck=SPV_NEW):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-dx-layout -layout-rules-version 202c

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

RWStructuredBuffer<U> sb;
RWStructuredBuffer<U, ScalarDataLayout> sbScalar;
StructuredBuffer<Q*> pointers;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    output[0] = sb[tid.x].w;
    output[1] = sbScalar[tid.x].w;
    output[2] = pointers[0]->v.y;
}

// The natural structured buffer keeps `U.w` at 12 and `U` at 16 bytes in reflection either way.
// Without 202c, the `ScalarDataLayout` buffer has the same numbers, since unrounded scalar layout
// equals natural layout; with 202c it is reflected with rounded scalar layout, and the pointee
// follows 202c.
// REFL_OLD: "name": "sb"
// REFL_OLD: "name": "w"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 12, "size": 4
// REFL_OLD: {"kind": "uniform", "value": 16, "alignment": 8}
// REFL_OLD: "name": "sbScalar"
// REFL_OLD: "name": "w"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 12, "size": 4
// REFL_OLD: {"kind": "uniform", "value": 16, "alignment": 8}
// REFL_NEW: "name": "sb"
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 12, "size": 4
// REFL_NEW: {"kind": "uniform", "value": 16, "alignment": 8}
// REFL_NEW: "name": "sbScalar"
// REFL_NEW: "name": "w"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 16, "size": 4
// REFL_NEW: {"kind": "uniform", "value": 24, "alignment": 8}
// REFL_OLD: "name": "pointers"
// REFL_OLD: "name": "v"
// REFL_OLD: "binding": {"kind": "uniform", "offset": 16, "size": 12
// REFL_NEW: "name": "pointers"
// REFL_NEW: "name": "v"
// REFL_NEW: "binding": {"kind": "uniform", "offset": 20, "size": 12

// The natural structured buffer keeps `U.w` at 12 and a 16-byte stride either way.
// SPV_OLD: OpMemberDecorate %[[SB:U_natural[_0-9]*]] 1 Offset 12
// SPV_OLD-NEXT: OpDecorate %_ptr_StorageBuffer_[[SB]] ArrayStride 16
// SPV_NEW: OpMemberDecorate %[[SB:U_natural[_0-9]*]] 1 Offset 12
// SPV_NEW-NEXT: OpDecorate %_ptr_StorageBuffer_[[SB]] ArrayStride 16

// With 202c the `ScalarDataLayout` buffer rounds, and the pointer to its elements uses the same
// 24-byte stride as its runtime array.
// SPV_NEW: OpMemberDecorate %U_scalar_rounded 1 Offset 16
// SPV_NEW-NEXT: OpDecorate %_ptr_StorageBuffer_U_scalar_rounded ArrayStride 24
// SPV_NEW-NEXT: OpDecorate %_runtimearr_U_scalar_rounded ArrayStride 24

// The pointee matches its reflection.
// SPV_OLD: OpMemberDecorate %Q_{{[a-z_0-9]+}} 2 Offset 16
// SPV_NEW: OpMemberDecorate %Q_scalar_rounded 2 Offset 20
