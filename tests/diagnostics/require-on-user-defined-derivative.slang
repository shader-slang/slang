// Regression test for https://github.com/shader-slang/slang/issues/8144
//
// A `[require(...)]` capability attribute on a user-defined derivative
// function must be honored: when the primal function (which references the
// derivative through a `[ForwardDerivative]` / `[BackwardDerivative]`
// attribute) is used on a target that does not provide the required
// capability, compilation should fail with E36107. The provenance notes
// should point at the derivative reference inside the attribute.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKFWD,non-exhaustive): -target hlsl -entry computeMainFwd -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKGEN,non-exhaustive): -target hlsl -entry computeMainGen -stage compute
// Positive counter-test: on a target that DOES provide the `spirv` capability,
// the same primal must compile with no E36107, proving the propagation only
// fires when the required capability is actually missing.
//TEST:SIMPLE(filecheck=CHECKOK): -target spirv-asm -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0], stride=16):out
RWStructuredBuffer<float> outputBuffer;

// --- Backward-derivative variant ---

[require(spirv)]
void testABwd(inout DifferentialPair<float> x, float result)
{
    x = {x.p, 0};
}

// The provenance note must point at the derivative reference inside the
// attribute (the function name), not the whole attribute.
[BackwardDerivative(testABwd)]
//CHECK:            ^^^^^^^^ see using of 'testABwd'
float testA(float x)
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
    // Calling the primal pulls in the capability requirements of its
    // user-defined backward derivative, which requires `spirv` and is therefore
    // unavailable for the `hlsl` target.
    outputBuffer[0] = testA(2.0);
}

// --- Forward-derivative variant (issue #8144 explicitly mentions this case) ---

[require(spirv)]
DifferentialPair<float> testBFwd(DifferentialPair<float> x)
{
    return DifferentialPair<float>(x.p * 2, x.d * 2);
}

[ForwardDerivative(testBFwd)]
//CHECKFWD:        ^^^^^^^^ see using of 'testBFwd'
float testB(float x)
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
    outputBuffer[0] = testB(2.0);
}

// --- Generic-derivative variant ---
//
// The derivative reference is wrapped in a generic application
// (`testCBwd<float>`), which exercises the `GenericAppExpr` stripping in
// `_getUserDefinedDerivativeFuncDecl`. The capability requirement must still be
// propagated and the provenance note must point at the generic function name.

[require(spirv)]
void testCBwd<T : __BuiltinFloatingPointType>(inout DifferentialPair<T> x, T.Differential result)
{
    x = diffPair(x.p, result);
}

[BackwardDerivative(testCBwd<float>)]
//CHECKGEN:         ^^^^^^^^ see using of 'testCBwd'
float testC(float x)
{
    return x * 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainGen(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECKGEN:
     ^^^^^^^^^^^^^^ error E36107
*/
{
    outputBuffer[0] = testC(2.0);
}

// On the `spirv` target the requirement is satisfied, so `computeMain`
// compiles to SPIR-V and no capability error is reported.
//CHECKOK: OpEntryPoint
//CHECKOK-NOT: E36107
