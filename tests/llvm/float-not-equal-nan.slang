// Coverage for floating-point `!=`/`==` on the CPU/LLVM target.
//
// `!=` must be the complement of `==`: true when either operand is NaN. Operands
// are loaded from a buffer so they are not compile-time constants — otherwise the
// comparison could be folded away before any `fcmp` is produced.
//
// The `-llvm` directive exercises the slang-llvm JIT path; the `-cpu` directive
// runs the emit-C++ path and pins the expected values as a control.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0x7FC00000 0x00000000 0x80000000 0x3F800000], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float nan = asfloat(inputBuffer[0]);
    float pzero = asfloat(inputBuffer[1]);
    float nzero = asfloat(inputBuffer[2]);
    float one = asfloat(inputBuffer[3]);

    uint i = 0;

    // Scalar `!=` truth values across NaN, finite, and signed-zero operands.
    // CHECK: 1
    outputBuffer[i++] = (nan != nan) ? 1u : 0u;
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
    outputBuffer[i++] = (one != pzero) ? 1u : 0u;
    // +0.0 == -0.0 in IEEE-754, so this must be false.
    // CHECK: 0
    outputBuffer[i++] = (pzero != nzero) ? 1u : 0u;

    // Scalar `==` for the same pairs (the complement partners of the above).
    // CHECK: 0
    outputBuffer[i++] = (nan == nan) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = (one == one) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = (pzero == nzero) ? 1u : 0u;

    // The invariant `(a != b) == !(a == b)`, asserted for every pair above.
    // CHECK: 1
    outputBuffer[i++] = ((nan != nan) == !(nan == nan)) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ((nan != one) == !(nan == one)) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ((one != nan) == !(one == nan)) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ((nan != pzero) == !(nan == pzero)) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ((nan != nzero) == !(nan == nzero)) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ((one != one) == !(one == one)) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ((one != pzero) == !(one == pzero)) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ((pzero != nzero) == !(pzero == nzero)) ? 1u : 0u;

    // Vector `!=`/`==` are per-lane with the same semantics.
    // Lanes: (nan,nan), (nan,one) NaN-left, (one,nan) reverse, (one,one) equal.
    float4 va1 = float4(nan, nan, one, one);
    float4 vb1 = float4(nan, one, nan, one);
    bool4 ne1 = va1 != vb1;
    bool4 eq1 = va1 == vb1;
    // CHECK: 1
    outputBuffer[i++] = ne1.x ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ne1.y ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ne1.z ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = ne1.w ? 1u : 0u;

    // Lanes: (one,pzero) unequal finite, (+0,-0), (nan,nan), (one,one).
    float4 va2 = float4(one, pzero, nan, one);
    float4 vb2 = float4(pzero, nzero, nan, one);
    bool4 ne2 = va2 != vb2;
    bool4 eq2 = va2 == vb2;
    // CHECK: 1
    outputBuffer[i++] = ne2.x ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = ne2.y ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = ne2.z ? 1u : 0u;
    // CHECK: 0
    outputBuffer[i++] = ne2.w ? 1u : 0u;

    // Per-lane complement invariant: every lane of `!=` differs from `==`.
    // CHECK: 1
    outputBuffer[i++] = all(ne1 != eq1) ? 1u : 0u;
    // CHECK: 1
    outputBuffer[i++] = all(ne2 != eq2) ? 1u : 0u;
}
