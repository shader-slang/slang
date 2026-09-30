// Under `-layout-rules-version 202c`, `sizeof` and `ByteAddressBuffer` `Load<T>`/`Store<T>` keep natural
// layout rather than following the rounded scalar layout of buffers, because `sizeof` is folded
// to a constant before the target is known and byte-address loads are lowered with natural
// offsets. This is provisional: whether they should round is an open question on #12714. DXC
// (v1.9.2602) does round `ByteAddressBuffer` struct loads under `-fvk-use-scalar-layout`, and reads
// `U.w` at byte 16.
//
// `U` is 16 bytes under natural layout and 24 when rounded, and `U.w` is at byte 12 or 16.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c

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
    output[2] = asuint(sb[tid.x].w);
    rwbab.Store<U>(0, sb[tid.x]);
}

// For contrast, the structured buffer rounds: `U.w` is at 16 and `U` has a 24-byte stride.
// CHECK: OpMemberDecorate %U_scalar_rounded 1 Offset 16
// CHECK: OpDecorate %_runtimearr_U_scalar_rounded ArrayStride 24

// `sizeof(U) + 1000` folds to 1016 (natural `sizeof(U)` of 16), not 1024.
// CHECK: %uint_1016 = OpConstant %uint 1016

// `bab.Load<U>(0).w` reads 32-bit word 3, which is byte 12, not word 4.
// CHECK: OpAccessChain %_ptr_StorageBuffer_float %bab %int_0 %uint_3

// `rwbab.Store<U>(0, ...)` writes `w` to 32-bit word 3 as well.
// CHECK: OpAccessChain %_ptr_StorageBuffer_float %rwbab{{(_[0-9]+)?}} %int_0 %uint_3
