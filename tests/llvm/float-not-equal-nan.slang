// Coverage for floating-point `!=`/`==` on the CPU/LLVM target.
//
// `!=` must be the complement of `==`: true when either operand is NaN. All
// operands are loaded from a buffer at runtime (asfloat of raw bit patterns) so
// the comparisons compile to genuine runtime `fcmp` instructions — a
// compile-time-constant NaN could be folded away before any comparison exists.
//
// The `-llvm` directive exercises the slang-llvm JIT path; the `-cpu` directive
// runs the emit-C++ path and pins the expected values as a control.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0x7FC00000 0x00000000 0x80000000 0x3F800000], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float nan = asfloat(inputBuffer[0]); // 0x7FC00000 : quiet NaN
    float pzero = asfloat(inputBuffer[1]); // 0x00000000 : +0.0
    float nzero = asfloat(inputBuffer[2]); // 0x80000000 : -0.0
    float one = asfloat(inputBuffer[3]); // 0x3F800000 : 1.0

    uint i = 0;

    // CHECK: 1
    outputBuffer[i++] = (nan != nan) ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = (nan == nan) ? 1u : 0u;

    // NaN vs finite, both operand orders.
    // CHECK: 1
    outputBuffer[i++] = (nan != one) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = (one != nan) ? 1u : 0u;

    // CHECK: 1
    outputBuffer[i++] = (nan != pzero) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = (nan != nzero) ? 1u : 0u;

    // CHECK: 0
    outputBuffer[i++] = (one != one) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = (one == one) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = (one != pzero) ? 1u : 0u;

    // +0.0 == -0.0 in IEEE-754, so `!=` is false — guards against a predicate
    // that is unconditionally true.
    // CHECK: 0
    outputBuffer[i++] = (pzero != nzero) ? 1u : 0u;

    // The defining invariant `x != y == !(x == y)`, for a NaN pair, an unequal
    // finite pair, and a NaN-vs-NaN pair.
    // CHECK: 1
    outputBuffer[i++] = ((nan != one) == !(nan == one)) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ((one != pzero) == !(one == pzero)) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ((nan != nan) == !(nan == nan)) ? 1u : 0u;

    // Vector `!=`/`==` are per-lane with the same semantics. Lanes cover
    // NaN-vs-NaN, finite-vs-NaN (reverse order), ±0, and unequal finite.
    float4 va = float4(nan, one, pzero, one);
    float4 vb = float4(nan, nan, nzero, pzero);
    bool4 ne = va != vb; // expect 1 1 0 1
    bool4 eq = va == vb; // expect 0 0 1 0
    // CHECK: 1
    outputBuffer[i++] = ne.x ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ne.y ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = ne.z ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ne.w ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = eq.x ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = eq.y ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = eq.z ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = eq.w ? 1u : 0u;

    // Per-lane complement invariant: every lane of `!=` differs from `==`.
    // CHECK: 1
    outputBuffer[i++] = all(ne != eq) ? 1u : 0u;
}
