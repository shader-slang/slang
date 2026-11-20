//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// Unequal conversion costs pick the strictly better type regardless of
// argument order. This is the positive companion to
// tests/diagnostics/generic-parameter-inference-type-tie.slang, which pins
// the non-bool equal-cost failure cases; mixed bool/number inference (the
// one-off bool-loses tie-break) and the explicit-type escape hatch are
// pinned in
// tests/diagnostics/generic-parameter-inference-explicit-type-argument.slang,
// whose diagnostic-test harness tolerates the implicit-conversion warnings
// those calls emit.

vector<T, 3> makevec3<T>(T a, T b, T c)
{
    return vector<T, 3>(a, b, c);
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // `uint` converts to `float` more cheaply than the reverse, so both
    // orders infer `T = float`; the fractional elements keep the inferred
    // type observable (inference of `T = uint` would truncate them to 0).
    let a = makevec3(0.5f, 2U, 0.5f);
    let b = makevec3(2U, 0.5f, 0.5f);
    outputBuffer[0] = a.x;
    // CHECK: 0.500000
    outputBuffer[1] = b.y;
    // CHECK: 0.500000
}
