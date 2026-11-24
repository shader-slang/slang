// A `__func_extension` declares a derivative for the function it names, in the way that a
// `[ForwardDerivativeOf]` or `[BackwardDerivativeOf]` attribute does. In a `__func_extension`,
// `fwd_diff(f)`, `bwd_diff(f)` or `__apply(f)` names the derivative of `f` that the body declares;
// it does not invoke one. A function with a reference parameter cannot be differentiated, so we
// reject such a function when an extension names it, as `differentiable.slang`
// does when an attribute names it. The extension is not checked against the function and gives it
// no derivative, so naming the function in `fwd_diff` or `bwd_diff` reports the missing
// derivative, and differentiating a caller of it adds no error.
//
// Before, an extension for `bwd_diff` or `__apply` of such a function reported an internal
// compiler error, and one for `fwd_diff` was accepted.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute -experimental-feature

groupshared float sharedFloat;

// --- A forward derivative ---

float refForwardExtended(__ref float a, float x)
//CHECK:                             ^ error E38038
{
    return a * x;
}

__func_extension fwd_diff(refForwardExtended)(__ref float a, DifferentialPair<float> x) -> DifferentialPair<float>
//CHECK:                  ^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return diffPair(a * x.p, a * x.d);
}

float groupSharedForwardExtended(groupshared float s, float x)
//CHECK:                                           ^ error E38038
{
    return s * x;
}

__func_extension fwd_diff(groupSharedForwardExtended)(groupshared float s, DifferentialPair<float> x) -> DifferentialPair<float>
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return diffPair(s * x.p, s * x.d);
}

// --- A backward derivative ---

float refBackwardExtended(__ref float a, float x)
//CHECK:                              ^ error E38038
{
    return a * x;
}

__func_extension bwd_diff(refBackwardExtended)(__ref float a, inout DifferentialPair<float> x, float dOut) -> void
//CHECK:                  ^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    x = diffPair(x.p, a * dOut);
}

float groupSharedBackwardExtended(groupshared float s, float x)
//CHECK:                                            ^ error E38038
{
    return s * x;
}

__func_extension bwd_diff(groupSharedBackwardExtended)(groupshared float s, inout DifferentialPair<float> x, float dOut) -> void
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    x = diffPair(x.p, s * dOut);
}

// --- A custom forward pass ---

// The context computes the derivative of `x` and takes `__none_t` in place of the derivative of
// the reference parameter, which has none.
struct ApplyContext
{
    float a;
    void operator()(__none_t na, out float dx, float dOut) { dx = a * dOut; }
}

float refApplyExtended(__ref float a, float x)
//CHECK:                           ^ error E38038
{
    return a * x;
}

__func_extension __apply(refApplyExtended)(__ref float a, float x) -> Tuple<float, ApplyContext>
//CHECK:                 ^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return makeTuple(a * x, ApplyContext(a));
}

float groupSharedApplyExtended(groupshared float s, float x)
//CHECK:                                         ^ error E38038
{
    return s * x;
}

__func_extension __apply(groupSharedApplyExtended)(groupshared float s, float x) -> Tuple<float, ApplyContext>
//CHECK:                 ^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return makeTuple(s * x, ApplyContext(s));
}

// --- A generic function ---

float genericForwardExtended<T : IFloat>(__ref float a, T x)
//CHECK:                                             ^ error E38038
{
    return a;
}

__func_extension<T : IFloat> fwd_diff(genericForwardExtended<T>)(__ref float a, DifferentialPair<T> x) -> DifferentialPair<float>
//CHECK:                              ^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return diffPair(a, 0.0);
}

// --- A function with a differentiability attribute ---

// The function is reported at the attribute, and the extension adds nothing.
[Differentiable]
/*CHECK:
 ^^^^^^^^^^^^^^ this requests a derivative; remove it if the function needs none
*/
float differentiableExtended(groupshared float s, float x)
//CHECK:                                       ^ error E38038
{
    return s * x;
}

__func_extension bwd_diff(differentiableExtended)(groupshared float s, inout DifferentialPair<float> x, float dOut) -> void
{
    x = diffPair(x.p, s * dOut);
}

// --- Differentiable callers ---

[Differentiable]
float callRefBackwardExtended(float x)
{
    float a = 1.0;
    return refBackwardExtended(a, x);
}

[Differentiable]
float callGroupSharedApplyExtended(float x)
{
    return groupSharedApplyExtended(sharedFloat, x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    sharedFloat = 1.0;
    float a = 1.0;
    var x = diffPair(1.0, 0.0);

    // Each function above has no derivative, so naming it reports the missing derivative.
    fwd_diff(refForwardExtended)(a, diffPair(1.0, 1.0));
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'fwd_diff' is not a member of 'refForwardExtended'.
*/
    fwd_diff(groupSharedForwardExtended)(sharedFloat, diffPair(1.0, 1.0));
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'fwd_diff' is not a member of 'groupSharedForwardExtended'.
*/
    bwd_diff(refBackwardExtended)(a, x, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'refBackwardExtended'.
*/
    bwd_diff(groupSharedBackwardExtended)(sharedFloat, x, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'groupSharedBackwardExtended'.
*/
    bwd_diff(refApplyExtended)(a, x, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'refApplyExtended'.
*/
    bwd_diff(groupSharedApplyExtended)(sharedFloat, x, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'groupSharedApplyExtended'.
*/
    fwd_diff(genericForwardExtended<float>)(a, diffPair(1.0, 1.0));
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'fwd_diff' is not a member of 'genericForwardExtended<float>'.
*/

    // A function with a differentiability attribute was reported at its declaration, so naming it
    // adds no error, and neither does differentiating a caller of a rejected function. The test
    // is exhaustive, so either would fail it.
    bwd_diff(differentiableExtended)(sharedFloat, x, 1.0);
    fwd_diff(callRefBackwardExtended)(diffPair(1.0, 1.0));
    bwd_diff(callGroupSharedApplyExtended)(x, 1.0);
}
