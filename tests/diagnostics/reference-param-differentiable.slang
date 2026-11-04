// A reference parameter -- `__ref`, `__ref_readonly`, `__ref_writeonly`, `const __ref`,
// `groupshared`, or `const groupshared` -- names storage that the caller, and for `groupshared`
// other invocations, may also access, and a derivative may access it a different number of times
// and in a different order than the function does. We therefore reject one on a function with any
// differentiability or derivative attribute. The error applies whatever the parameter's type,
// whether or not it is marked `no_diff`, and to interface requirements and methods. A function
// without an attribute is rejected in the same way when `fwd_diff` or `bwd_diff` names it, or when
// a `[ForwardDerivativeOf]` or `[BackwardDerivativeOf]` attribute names it as a primal, and its
// derivative is then not checked against it. A function that is neither differentiated nor given a
// derivative may take one.
//
// Before, backward differentiation of a caller of a user-defined backward derivative with a
// reference parameter could fail SPIR-V validation or report an internal compiler error, because
// the backward context held the reference.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute -experimental-feature

static const uint N = 8;
groupshared float sharedFloats[N];

// --- Rejected: each reference spelling ---

[Differentiable]
float refParam(__ref float a, float x)
//CHECK:                   ^ error E38038
//CHECK:                   ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

[Differentiable]
float refReadOnlyParam(__ref_readonly float a, float x)
//CHECK:                                    ^ error E38038
//CHECK:                                    ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

[Differentiable]
float refWriteOnlyParam(__ref_writeonly float a, float x)
//CHECK:                                      ^ error E38038
//CHECK:                                      ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    a = detach(x);
    return x;
}

[Differentiable]
float constRefParam(const __ref float a, float x)
//CHECK:                              ^ error E38038
//CHECK:                              ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

[Differentiable]
float groupSharedParam(uint tid, groupshared float s[N])
//CHECK:                                           ^ error E38038
//CHECK:                                           ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return s[tid];
}

[Differentiable]
float constGroupSharedParam(uint tid, const groupshared float s[N])
//CHECK:                                                      ^ error E38038
//CHECK:                                                      ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return s[tid];
}

// --- Rejected: marked `no_diff`, or with a type that has no derivative ---

[Differentiable]
float noDiffGroupShared(uint tid, no_diff groupshared float s[N], float x)
//CHECK:                                                    ^ error E38038
//CHECK:                                                    ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return s[tid] * x;
}

[Differentiable]
float noDiffRef(no_diff __ref float a, float x)
//CHECK:                            ^ error E38038
//CHECK:                            ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

[Differentiable]
float nonDifferentiableType(__ref int a, float x)
//CHECK:                              ^ error E38038
//CHECK:                              ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

// --- Rejected: each differentiability attribute, and with a user-defined derivative ---

[ForwardDifferentiable]
float forwardDifferentiable(__ref float a, float x)
//CHECK:                                ^ error E38038
//CHECK:                                ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

[BackwardDifferentiable]
float backwardDifferentiable(__ref float a, float x)
//CHECK:                                 ^ error E38038
//CHECK:                                 ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

// --- Rejected: each derivative attribute, on the function that carries it ---

[ForwardDerivative(forwardDerivativeOfNamed)]
float forwardDerivativeNamed(__ref float a, float x)
//CHECK:                                 ^ error E38038
//CHECK:                                 ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

DifferentialPair<float> forwardDerivativeOfNamed(__ref float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p, a * x.d);
}

[BackwardDerivative(backwardDerivativeOfNamed)]
float backwardDerivativeNamed(__ref float a, float x)
//CHECK:                                  ^ error E38038
//CHECK:                                  ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

void backwardDerivativeOfNamed(__ref float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, a * dOut);
}

[TreatAsDifferentiable]
float treatedAsDifferentiable(__ref float a, float x)
//CHECK:                                  ^ error E38038
//CHECK:                                  ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

// --- Rejected: a primal named by a `...DerivativeOf` attribute, and its derivative ---

float forwardPrimal(__ref float a, float x)
//CHECK:                        ^ error E38038
//CHECK:                        ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

[ForwardDerivativeOf(forwardPrimal)]
DifferentialPair<float> forwardDerivativeOf(__ref float a, DifferentialPair<float> x)
//CHECK:                                                ^ error E38038
//CHECK:                                                ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return diffPair(a * x.p, a * x.d);
}

float backwardPrimal(uint tid, const groupshared float s[N], float x)
//CHECK:                                               ^ error E38038
//CHECK:                                               ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return s[tid] * x;
}

[BackwardDerivativeOf(backwardPrimal)]
void backwardDerivativeOf(uint tid, no_diff const groupshared float s[N], inout DifferentialPair<float> x, float dOut)
//CHECK:                                                            ^ error E38038
//CHECK:                                                            ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    x = diffPair(x.p, s[tid] * dOut);
}

float byValueDerivativePrimal(__ref float a, float x)
//CHECK:                                  ^ error E38038
//CHECK:                                  ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

[BackwardDerivativeOf(byValueDerivativePrimal)]
void byValueDerivativeOf(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, a * dOut);
}

// --- Rejected: an interface requirement and a method ---

interface IScale
{
    [Differentiable]
    float scale(__ref float a, float x);
//CHECK:                    ^ error E38038
//CHECK:                    ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
}

struct Scaler
{
    [Differentiable]
    float scale(__ref float a, float x)
//CHECK:                    ^ error E38038
//CHECK:                    ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
    {
        return a * x;
    }
}

// --- Rejected: a function without an attribute, named by `fwd_diff` or `bwd_diff` ---

float forwardDifferentiated(__ref float a, float x)
//CHECK:                                ^ error E38038
//CHECK:                                ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
{
    return a * x;
}

float backwardDifferentiated(__ref float a, float x)
//CHECK:                                 ^ error E38038
//CHECK:                                 ^ if the function needs no derivative, remove the differentiability or derivative attribute that applies to it; otherwise pass the value as an ordinary parameter
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

    // Naming a function with a reference parameter in `fwd_diff` or `bwd_diff` reports the
    // parameter once, at its declaration, whether or not the function has an attribute.
    float a = 1.0;
    fwd_diff(forwardDifferentiated)(a, diffPair(1.0, 1.0));
    var x = diffPair(1.0, 0.0);
    bwd_diff(backwardDifferentiated)(a, x, 1.0);
    fwd_diff(refParam)(a, diffPair(1.0, 1.0));
}
