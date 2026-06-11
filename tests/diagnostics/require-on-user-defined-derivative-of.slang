// Regression test for https://github.com/shader-slang/slang/issues/11551
//
// A user-defined derivative may be linked to its primal through the *inverse*
// placement: `[ForwardDerivativeOf(primal)]` / `[BackwardDerivativeOf(primal)]`
// on the derivative itself. That placement records a derivative *association* on
// the primal rather than a `[ForwardDerivative]` / `[BackwardDerivative]` modifier
// on the primal.
//
// A `[require(...)]` capability attribute on such an inverse-placed derivative must
// be honored: when the primal is used on a target that does not provide the required
// capability, compilation should fail with E36107. The provenance note should point
// at the derivative function that declares the requirement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKFWD,non-exhaustive): -target hlsl -entry computeMainFwd -stage compute

//TEST_INPUT:ubuffer(data=[0], stride=16):out
RWStructuredBuffer<float> outputBuffer;

// --- Backward-derivative variant (inverse placement) ---

// The `[require(spirv)]` and the primal linkage both live on the derivative; the
// provenance note must point here (the derivative function name), not at the primal.
[require(spirv)]
[BackwardDerivativeOf(testC)]
void testCBwd(inout DifferentialPair<float> x, float dResult)
/*CHECK:
     ^^^^^^^^ see using of 'testCBwd'
*/
{
    x = diffPair(x.p, 0.0);
}

float testC(float x)
{
    return x * 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECK:
     ^^^^^^^^^^^ error E36107
*/
{
    // Using the primal pulls in the capability requirements of its inverse-placed
    // backward derivative, which requires `spirv` and is unavailable for `hlsl`.
    outputBuffer[0] = testC(2.0);
}

// --- Forward-derivative variant (inverse placement) ---

[require(spirv)]
[ForwardDerivativeOf(testD)]
DifferentialPair<float> testDFwd(DifferentialPair<float> x)
//CHECKFWD:             ^^^^^^^^ see using of 'testDFwd'
{
    return DifferentialPair<float>(x.p * 2, x.d * 2);
}

float testD(float x)
{
    return x * 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainFwd(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECKFWD:
     ^^^^^^^^^^^^^^ error E36107
*/
{
    outputBuffer[0] = testD(2.0);
}
