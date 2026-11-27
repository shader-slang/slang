// Regression test for https://github.com/shader-slang/slang/issues/13200.
//
// Runtime pointer-sized integers (`intptr_t`/`kIROp_IntPtrType` and its unsigned
// sibling `uintptr_t`/`kIROp_UIntPtrType`) are ordinary scalars that lower, on a
// 64-bit-pointer SPIR-V target, to a 64-bit OpTypeInt: signed for `intptr_t`,
// unsigned for `uintptr_t`.
//
// The test stays off `sizeof`/explicit layout on purpose: that path reaches the
// unrelated, still-open SizeAndAlignment limitation (#8480).

//TEST:SIMPLE(filecheck=CHECK_S): -target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -validate-ir
//TEST:SIMPLE(filecheck=CHECK_U): -target spirv-asm -entry computeMainU -stage compute -emit-spirv-directly -validate-ir

// A pointer-sized specialization constant exercises the OpSpecConstant literal path. It is used
// only by the signed entry point because that literal path is signedness-independent, so covering
// it once is enough.
[SpecializationConstant]
const intptr_t iterCount = 4;
// The negative spec constant pins the 64-bit literal selection: a 32-bit truncation of
// the bits would render as 4294967295 instead of -1.
[SpecializationConstant]
const intptr_t negOffset = -1;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid: SV_DispatchThreadID, uniform RWStructuredBuffer<float> outp)
{
    float acc = 0.0f;
    intptr_t count = intptr_t(tid.y) + iterCount + negOffset;
    for (intptr_t k = 0; k < count; k++)
    {
        acc = acc + (float)k;
    }
    // Division and remainder on runtime pointer-sized values must select the signed
    // SPIR-V ops; the operands come from `tid` so the operations cannot fold away.
    intptr_t quot = (count + intptr_t(tid.x)) / (intptr_t(tid.y) + 3);
    intptr_t rem = (count + intptr_t(tid.x)) % (intptr_t(tid.y) + 3);
    // The 33-bit literal needs two 32-bit words, pinning the 64-bit OpConstant path.
    intptr_t big = intptr_t(0x100000000) + intptr_t(tid.z);
    outp[0] = acc + (float)quot + (float)rem + (float)big;
}

// CHECK_S-DAG: OpTypeInt 64 1
// CHECK_S-DAG: OpSpecConstant %long 4
// The negative spec constant distinguishes the 64-bit literal path from a 32-bit truncation.
// CHECK_S-DAG: OpSpecConstant %long -1
// The 33-bit literal distinguishes the 64-bit OpConstant path from a 32-bit truncation,
// which would render it as 0.
// CHECK_S-DAG: OpConstant %long 4294967296
// Division and remainder on signed pointer-sized values select the signed ops.
// CHECK_S-DAG: OpSDiv
// CHECK_S-DAG: OpSRem
// The uint->intptr_t widen goes through a 64-bit unsigned intermediate (%ulong) before it is
// reinterpreted as signed, so the widening instruction is an unsigned OpUConvert.
// CHECK_S-DAG: OpUConvert %ulong
// The loop compares signed pointer-sized values, so it must use the signed OpSLessThan.
// CHECK_S-DAG: OpSLessThan

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMainU(uint3 tid: SV_DispatchThreadID, uniform RWStructuredBuffer<float> outp)
{
    float acc = 0.0f;
    uintptr_t count = uintptr_t(tid.y);
    for (uintptr_t k = 0; k < count; k++)
    {
        acc = acc + (float)k;
    }
    outp[0] = acc;
}

// CHECK_U-DAG: OpTypeInt 64 0
// The loop compares unsigned pointer-sized values, so it must use the unsigned OpULessThan.
// CHECK_U-DAG: OpULessThan
