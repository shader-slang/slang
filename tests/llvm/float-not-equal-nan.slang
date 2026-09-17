// Regression test for shader-slang/slang#13144.
//
// The CPU/LLVM backend lowered a floating-point `!=` to the *ordered*
// not-equal predicate (LLVM `FCMP_ONE`), which is false whenever an operand is
// NaN. `!=` must be the logical complement of `==`, i.e. true when either
// operand is NaN. The fix uses the unordered predicate `FCMP_UNE` in
// LLVMBuilder::emitCompareOp (source/slang-llvm/slang-llvm-builder.cpp).
//
// All operands are loaded from a buffer at runtime (asfloat of raw bit
// patterns) so the comparisons compile to genuine runtime `fcmp` instructions;
// a compile-time-constant NaN could be folded away and would mask the bug.
//
// The `-llvm` directive exercises the affected slang-llvm JIT path (red before
// the fix, green after). The `-cpu` directive runs the emit-C++ path, which is
// already correct and pins the expected values as a control.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0x7FC00000 0x00000000 0x80000000 0x3F800000], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float nan = asfloat(inputBuffer[0]); // 0x7FC00000 : quiet NaN
    float pzero = asfloat(inputBuffer[1]); // 0x00000000 : +0.0
    float nzero = asfloat(inputBuffer[2]); // 0x80000000 : -0.0
    float one = asfloat(inputBuffer[3]); // 0x3F800000 : 1.0

    uint i = 0;

    // NaN != NaN is true; NaN == NaN is false.
    // CHECK: 1
    outputBuffer[i++] = (nan != nan) ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = (nan == nan) ? 1u : 0u;

    // NaN != finite is true in both operand orders.
    // CHECK: 1
    outputBuffer[i++] = (nan != one) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = (one != nan) ? 1u : 0u;

    // NaN != +0.0 and NaN != -0.0 are both true.
    // CHECK: 1
    outputBuffer[i++] = (nan != pzero) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = (nan != nzero) ? 1u : 0u;

    // Non-NaN cases must stay correct: equal finite values are not `!=`, and
    // +0.0 == -0.0 in IEEE-754 so +0.0 != -0.0 is false (guards against a
    // predicate that is unconditionally true).
    // CHECK: 0
    outputBuffer[i++] = (one != one) ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = (pzero != nzero) ? 1u : 0u;

    // The defining invariant: `x != y` equals `!(x == y)` for a NaN operand.
    // CHECK: 1
    outputBuffer[i++] = ((nan != one) == !(nan == one)) ? 1u : 0u;

    // Vector `!=` is per-lane with the same semantics.
    float4 va = float4(nan, one, pzero, nan);
    float4 vb = float4(nan, one, nzero, one);
    bool4 ne = va != vb; // lanes: NaN!=NaN, 1!=1, +0!=-0, NaN!=1
    // CHECK: 1
    outputBuffer[i++] = ne.x ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = ne.y ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = ne.z ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ne.w ? 1u : 0u;
}
