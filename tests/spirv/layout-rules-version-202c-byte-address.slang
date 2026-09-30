// `-layout-rules-version 202c` also rounds the layout of structs read and written through a
// `ByteAddressBuffer` with `Load<T>`/`Store<T>`, as DXC does, so that `bab.Load<U>` reads what a
// scalar-layout buffer of `U` holds. `sizeof` stays natural, because it is folded to a constant
// before the target is known. The `sizeof` checks record that behavior provisionally, not as a
// settled design.
//
// `U` is 16 bytes and `U.w` is at byte 12 (32-bit word 3) without rounding. With 202c `U` is 24
// bytes and `U.w` is at byte 16 (word 4), so in `U[2]` the second element's `w` moves from byte 28
// to byte 40.

//TEST:SIMPLE(filecheck=OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout
//TEST:SIMPLE(filecheck=NEW):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c
//TEST:SIMPLE(filecheck=NOFLAG):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -layout-rules-version 202c

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

ByteAddressBuffer bab;
RWByteAddressBuffer rwbab;
RWStructuredBuffer<U> sb;
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    output[0] = sizeof(U) + 1000;
    output[1] = asuint(bab.Load<U>(0).w);
    output[2] = asuint(bab.Load<U[2]>(tid.x * 64)[1].w);
    output[3] = asuint(sb[tid.x].w);
    rwbab.Store<U>(0, sb[tid.x]);
}

// OLD: %uint_1016 = OpConstant %uint 1016
// NEW: %uint_1016 = OpConstant %uint 1016

// OLD: %uint_28 = OpConstant %uint 28
// OLD: OpAccessChain %_ptr_StorageBuffer_float %bab %int_0 %uint_3
// OLD: OpIAdd %uint %{{[0-9]+}} %uint_28
// OLD: OpAccessChain %_ptr_StorageBuffer_float %rwbab{{(_[0-9]+)?}} %int_0 %uint_3

// NEW: %uint_40 = OpConstant %uint 40
// NEW: OpAccessChain %_ptr_StorageBuffer_float %bab %int_0 %uint_4
// NEW: OpIAdd %uint %{{[0-9]+}} %uint_40
// NEW: OpAccessChain %_ptr_StorageBuffer_float %rwbab{{(_[0-9]+)?}} %int_0 %uint_4

// Byte-address access follows 202c without `-fvk-use-scalar-layout` as well.
// NOFLAG: %uint_40 = OpConstant %uint 40
// NOFLAG: OpAccessChain %_ptr_StorageBuffer_float %bab %int_0 %uint_4
// NOFLAG: OpIAdd %uint %{{[0-9]+}} %uint_40
// NOFLAG: OpAccessChain %_ptr_StorageBuffer_float %rwbab{{(_[0-9]+)?}} %int_0 %uint_4
