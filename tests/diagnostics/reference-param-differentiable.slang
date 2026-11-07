// A reference parameter -- `__ref`, `__ref_readonly`, `__ref_writeonly`, `const __ref`,
// `groupshared`, or `const groupshared` -- names storage that the caller, and for `groupshared`
// other invocations, may also access, and a derivative may access it a different number of times
// and in a different order than the function does. We therefore reject one on a function with any
// differentiability or derivative attribute. The error applies whatever the parameter's type,
// whether or not it is marked `no_diff`, and to interface requirements and methods. A function
// without an attribute is rejected in the same way when a `[ForwardDerivativeOf]` or
// `[BackwardDerivativeOf]` attribute names it as a primal, and its derivative is then not checked
// against it. Naming a function without an attribute in `fwd_diff` or `bwd_diff` reports the
// missing derivative, as it does for a function without a reference parameter. A function that is
// neither differentiated nor given a derivative may take one.
//
// Before, backward differentiation of a caller of a user-defined backward derivative with a
// reference parameter could fail SPIR-V validation or report an internal compiler error, because
// the backward context held the reference.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute -experimental-feature

static const uint N = 8;
groupshared float sharedFloats[N];

// --- Rejected: each reference spelling ---

[Differentiable]
/*CHECK:
 ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float refParam(__ref float a, float x)
//CHECK:                   ^ error E38038
{
    return a * x;
}

[Differentiable]
/*CHECK:
 ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float refReadOnlyParam(__ref_readonly float a, float x)
//CHECK:                                    ^ error E38038
{
    return a * x;
}

[Differentiable]
/*CHECK:
 ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float refWriteOnlyParam(__ref_writeonly float a, float x)
//CHECK:                                      ^ error E38038
{
    a = detach(x);
    return x;
}

[Differentiable]
/*CHECK:
 ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float constRefParam(const __ref float a, float x)
//CHECK:                              ^ error E38038
{
    return a * x;
}

[Differentiable]
/*CHECK:
 ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float groupSharedParam(uint tid, groupshared float s[N])
//CHECK:                                           ^ error E38038
{
    return s[tid];
}

[Differentiable]
/*CHECK:
 ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float constGroupSharedParam(uint tid, const groupshared float s[N])
//CHECK:                                                      ^ error E38038
{
    return s[tid];
}

// --- Rejected: marked `no_diff`, or with a type that has no derivative ---

[Differentiable]
/*CHECK:
 ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float noDiffGroupShared(uint tid, no_diff groupshared float s[N], float x)
//CHECK:                                                    ^ error E38038
{
    return s[tid] * x;
}

[Differentiable]
/*CHECK:
 ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float noDiffRef(no_diff __ref float a, float x)
//CHECK:                            ^ error E38038
{
    return a * x;
}

[Differentiable]
/*CHECK:
 ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float nonDifferentiableType(__ref int a, float x)
//CHECK:                              ^ error E38038
{
    return a * x;
}

// --- Rejected: each differentiability attribute, and with a user-defined derivative ---

[ForwardDifferentiable]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float forwardDifferentiable(__ref float a, float x)
//CHECK:                                ^ error E38038
{
    return a * x;
}

[BackwardDifferentiable]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float backwardDifferentiable(__ref float a, float x)
//CHECK:                                 ^ error E38038
{
    return a * x;
}

// --- Rejected: each derivative attribute, on the function that carries it ---

[ForwardDerivative(forwardDerivativeOfNamed)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float forwardDerivativeNamed(__ref float a, float x)
//CHECK:                                 ^ error E38038
{
    return a * x;
}

DifferentialPair<float> forwardDerivativeOfNamed(__ref float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p, a * x.d);
}

[BackwardDerivative(backwardDerivativeOfNamed)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float backwardDerivativeNamed(__ref float a, float x)
//CHECK:                                  ^ error E38038
{
    return a * x;
}

void backwardDerivativeOfNamed(__ref float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, a * dOut);
}

[TreatAsDifferentiable]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float treatedAsDifferentiable(__ref float a, float x)
//CHECK:                                  ^ error E38038
{
    return a * x;
}

// --- Rejected: a primal named by a `...DerivativeOf` attribute, and its derivative ---

float forwardPrimal(__ref float a, float x)
//CHECK:                        ^ error E38038
{
    return a * x;
}

[ForwardDerivativeOf(forwardPrimal)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^ this declares the function as a user-defined derivative
                     ^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
DifferentialPair<float> forwardDerivativeOf(__ref float a, DifferentialPair<float> x)
//CHECK:                                                ^ error E38044
//CHECK:                                                ^ consider accessing the 'groupshared' variable directly instead of passing it as a parameter
{
    return diffPair(a * x.p, a * x.d);
}

float backwardPrimal(uint tid, const groupshared float s[N], float x)
//CHECK:                                               ^ error E38038
{
    return s[tid] * x;
}

[BackwardDerivativeOf(backwardPrimal)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^^ this declares the function as a user-defined derivative
                      ^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
void backwardDerivativeOf(uint tid, no_diff const groupshared float s[N], inout DifferentialPair<float> x, float dOut)
//CHECK:                                                            ^ error E38044
//CHECK:                                                            ^ consider accessing the 'groupshared' variable directly instead of passing it as a parameter
{
    x = diffPair(x.p, s[tid] * dOut);
}

float byValueDerivativePrimal(__ref float a, float x)
//CHECK:                                  ^ error E38038
{
    return a * x;
}

[BackwardDerivativeOf(byValueDerivativePrimal)]
/*CHECK:
                      ^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
void byValueDerivativeOf(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, a * dOut);
}

// --- Rejected: an interface requirement and a method ---

interface IScale
{
    [Differentiable]
/*CHECK:
     ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
    float scale(__ref float a, float x);
//CHECK:                    ^ error E38038
}

struct Scaler
{
    [Differentiable]
/*CHECK:
     ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
    float scale(__ref float a, float x)
//CHECK:                    ^ error E38038
    {
        return a * x;
    }
}

// --- Rejected at the use: a function without an attribute, named by `fwd_diff` or `bwd_diff` ---

float forwardDifferentiated(__ref float a, float x)
{
    return a * x;
}

float backwardDifferentiated(__ref float a, float x)
{
    return a * x;
}

// --- Accepted: a function that is not differentiable ---

float notDifferentiable(__ref float a, groupshared float s[N], uint tid)
{
    return a * s[tid];
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    sharedFloats[tid.x] = float(tid.x);

    // A function without an attribute has no derivative, so naming it in `fwd_diff` or `bwd_diff`
    // reports the missing derivative, whether or not it has a reference parameter.
    float a = 1.0;
    fwd_diff(forwardDifferentiated)(a, diffPair(1.0, 1.0));
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'fwd_diff' is not a member of 'forwardDifferentiated'.
*/
    var x = diffPair(1.0, 0.0);
    bwd_diff(backwardDifferentiated)(a, x, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'backwardDifferentiated'.
*/

    // A primal that a `...DerivativeOf` attribute names was reported at that attribute, which
    // attaches no derivative to it, so naming it reports the missing derivative too.
    fwd_diff(forwardPrimal)(a, diffPair(1.0, 1.0));
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'fwd_diff' is not a member of 'forwardPrimal'.
*/

    // A function with a differentiability attribute is already reported at its declaration, with
    // a note at the attribute, so naming it in `fwd_diff` or `bwd_diff` adds neither a second
    // error nor a note here. The test is exhaustive, so either would fail it.
    fwd_diff(refParam)(a, diffPair(1.0, 1.0));
    bwd_diff(refParam)(a, x, 1.0);
}
