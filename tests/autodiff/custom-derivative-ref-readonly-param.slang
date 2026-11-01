//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature

// A custom forward derivative may take a `__ref_readonly` parameter matching the primal's.
// Checking the attribute synthesizes a `fwd_diff` witness that forwards each of its parameters
// to the derivative, so a forwarded read-only reference must be accepted by the derivative's
// `__ref_readonly` parameter. Neither function is differentiated here; the test checks that
// both attribute spellings are accepted and that the primal runs.

[ForwardDerivative(scaleFwd)]
float scale(__ref_readonly int k, float x)
{
    return x * k;
}

DifferentialPair<float> scaleFwd(__ref_readonly int k, DifferentialPair<float> x)
{
    return diffPair(x.p * k, x.d * k);
}

float offset(__ref_readonly int k, float x)
{
    return x + k;
}

[ForwardDerivativeOf(offset)]
DifferentialPair<float> offsetFwd(__ref_readonly int k, DifferentialPair<float> x)
{
    return diffPair(x.p + k, x.d);
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int k = 3;
    // CHECK: 6.0
    outputBuffer[0] = scale(k, 2.0);
    // CHECK: 5.0
    outputBuffer[1] = offset(k, 2.0);
}
