// Regression test for https://github.com/shader-slang/slang/issues/8144 and
// https://github.com/shader-slang/slang/issues/11859.
//
// A `[require(...)]` capability attribute on a user-defined derivative function
// (linked to its primal through a `[ForwardDerivative]` / `[BackwardDerivative]`
// attribute) is honored at the *differentiation use-site*: differentiating the
// primal (`fwd_diff` / `bwd_diff`) on a target that does not provide the required
// capability fails with E36107, with a provenance note pointing at the derivative
// (#8144). A plain, non-differentiated call to the primal does NOT inherit the
// derivative's requirement and compiles for any target (#11859).

// #8144: differentiating on a target lacking the capability still fails.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry computeMainDiffBwd -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKFWD,non-exhaustive): -target hlsl -entry computeMainDiffFwd -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKGEN,non-exhaustive): -target hlsl -entry computeMainDiffGen -stage compute
// #11859: plain (non-differentiated) primal calls compile for hlsl (no E36107).
//TEST:SIMPLE(filecheck=CHECKPLAIN): -target hlsl -entry computeMainPlain -stage compute
// Positive counter-test: differentiating on a target that DOES provide `spirv`
// compiles with no E36107.
//TEST:SIMPLE(filecheck=CHECKOK): -target spirv-asm -entry computeMainDiffBwd -stage compute

//TEST_INPUT:ubuffer(data=[0], stride=16):out
RWStructuredBuffer<float> outputBuffer;

// --- Backward-derivative variant ---

[require(spirv)]
void testABwd(inout DifferentialPair<float> x, float result)
// Provenance note names the derivative; matched as a substring (its column varies with the
// differentiation form — see the note in require-on-user-defined-derivative-of.slang).
//CHECK: see using of 'testABwd'
{
    x = {x.p, 0};
}

[BackwardDerivative(testABwd)]
float testA(float x)
{
    return x * 2;
}

// --- Forward-derivative variant (issue #8144 explicitly mentions this case) ---

[require(spirv)]
DifferentialPair<float> testBFwd(DifferentialPair<float> x)
//CHECKFWD: see using of 'testBFwd'
{
    return DifferentialPair<float>(x.p * 2, x.d * 2);
}

[ForwardDerivative(testBFwd)]
float testB(float x)
{
    return x * 2;
}

// --- Generic-derivative variant ---
//
// The derivative reference is wrapped in a generic application (`testCBwd<float>`),
// which exercises the `GenericAppExpr` stripping in `_getFuncDeclFromExpr`.

[require(spirv)]
void testCBwd<T : __BuiltinFloatingPointType>(inout DifferentialPair<T> x, T.Differential result)
/*CHECKGEN:
     ^^^^^^^^ see using of 'testCBwd'
*/
{
    x = diffPair(x.p, result);
}

[BackwardDerivative(testCBwd<float>)]
float testC(float x)
{
    return x * 2;
}

// Differentiating the primal on `hlsl` pulls in the `spirv` requirement of its
// user-defined derivative, which is unavailable for `hlsl`: E36107.

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainDiffBwd(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECK:
     ^^^^^^^^^^^^^^^^^^ error E36107
*/
{
    var dp = diffPair(2.0, 0.0);
    bwd_diff(testA)(dp, 1.0);
    outputBuffer[0] = dp.d;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainDiffFwd(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECKFWD:
     ^^^^^^^^^^^^^^^^^^ error E36107
*/
{
    outputBuffer[0] = fwd_diff(testB)(diffPair(2.0, 1.0)).d;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainDiffGen(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECKGEN:
     ^^^^^^^^^^^^^^^^^^ error E36107
*/
{
    var dp = diffPair(2.0, 0.0);
    bwd_diff(testC)(dp, 1.0);
    outputBuffer[0] = dp.d;
}

// A plain, non-differentiated call to each primal must compile for `hlsl` — the
// derivatives' `[require(spirv)]` does not reach a non-differentiated use (#11859).

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainPlain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    //CHECKPLAIN-NOT: 36107
    outputBuffer[0] = testA(2.0) + testB(2.0) + testC(2.0);
}

// On the `spirv` target the requirement is satisfied, so differentiating compiles.
//CHECKOK: OpEntryPoint
//CHECKOK-NOT: E36107
