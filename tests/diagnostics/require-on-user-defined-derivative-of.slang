// Regression test for https://github.com/shader-slang/slang/issues/11551 and
// https://github.com/shader-slang/slang/issues/11859.
//
// A user-defined derivative may be linked to its primal through the *inverse*
// placement: `[ForwardDerivativeOf(primal)]` / `[BackwardDerivativeOf(primal)]`
// on the derivative itself. That placement records a derivative *association* on
// the primal rather than a `[ForwardDerivative]` / `[BackwardDerivative]` modifier
// on the primal.
//
// A `[require(...)]` capability attribute on such a derivative is honored at the
// *differentiation use-site*: differentiating the primal (`fwd_diff` / `bwd_diff`)
// on a target that does not provide the required capability fails with E36107, and
// the provenance note points at the derivative function that declares the
// requirement (#11551). A plain, non-differentiated call to the primal does NOT
// inherit the derivative's requirement and compiles for any target (#11859).

// #11551: differentiating on a target lacking the capability still fails.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKBWD,non-exhaustive): -target hlsl -entry computeMainDiffBwd -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKFWD,non-exhaustive): -target hlsl -entry computeMainDiffFwd -stage compute
// #11859: a plain non-differentiated primal call compiles for hlsl (no E36107).
//TEST:SIMPLE(filecheck=CHECKPLAIN): -target hlsl -entry computeMainPlain -stage compute

//TEST_INPUT:ubuffer(data=[0], stride=4):out
RWStructuredBuffer<float> outputBuffer;

// --- Backward-derivative variant (inverse placement) ---

// The `[require(spirv)]` and the primal linkage both live on the derivative; the
// provenance note must point here (the derivative function name).
[require(spirv)]
[BackwardDerivativeOf(testC)]
void testCBwd(inout DifferentialPair<float> x, float dResult)
// The provenance note names the derivative that declares the requirement. Matched as a
// substring rather than by caret column: the "see using" note attaches to the derivative
// for a `bwd_diff` statement but to the use-site for an inline `fwd_diff` expression, so a
// fixed column is not portable across the two placements.
//CHECKBWD: see using of 'testCBwd'
{
    x = diffPair(x.p, 0.0);
}

float testC(float x)
{
    return x * 2;
}

// --- Forward-derivative variant (inverse placement) ---

[require(spirv)]
[ForwardDerivativeOf(testD)]
DifferentialPair<float> testDFwd(DifferentialPair<float> x)
//CHECKFWD: see using of 'testDFwd'
{
    return DifferentialPair<float>(x.p * 2, x.d * 2);
}

float testD(float x)
{
    return x * 2;
}

// Differentiating the primal on `hlsl` pulls in the `spirv` requirement of its
// inverse-placed derivative, which is unavailable for `hlsl`: E36107.

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainDiffBwd(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECKBWD:
     ^^^^^^^^^^^^^^^^^^ error E36107
*/
{
    var dp = diffPair(2.0, 0.0);
    bwd_diff(testC)(dp, 1.0);
    outputBuffer[0] = dp.d;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainDiffFwd(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECKFWD:
     ^^^^^^^^^^^^^^^^^^ error E36107
*/
{
    outputBuffer[0] = fwd_diff(testD)(diffPair(2.0, 1.0)).d;
}

// A plain, non-differentiated call to the primal must compile for `hlsl` — the
// derivative's `[require(spirv)]` does not reach a non-differentiated use (#11859).

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainPlain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    //CHECKPLAIN-NOT: 36107
    outputBuffer[0] = testC(2.0) + testD(2.0);
}

// --- Negative case: the [require] gate must NOT propagate when the derivative
// carries no [require], even when the primal is differentiated. This guards the
// gate that keeps the core module's [require]-free derivative families from
// imposing requirements on all-targets builtin primals.

//TEST:SIMPLE(filecheck=CHECKNOREQ): -target hlsl -entry computeMainNoReqDiff -stage compute

[require(spirv)]
float spirvOnlyHelper(float x)
{
    return x;
}

// No [require] on the derivative itself, although its body transitively needs `spirv`.
[BackwardDerivativeOf(testE)]
void testEBwd(inout DifferentialPair<float> x, float dResult)
{
    x = diffPair(x.p, spirvOnlyHelper(0.0));
}

float testE(float x)
{
    return x * 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainNoReqDiff(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // testEBwd has no [require], so its requirements are NOT propagated even when
    // testE is differentiated; compiling for `hlsl` must not raise E36107.
    //CHECKNOREQ-NOT: 36107
    var dp = diffPair(2.0, 0.0);
    bwd_diff(testE)(dp, 1.0);
    outputBuffer[0] = dp.d;
}
