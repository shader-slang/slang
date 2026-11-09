//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

// A `__func_extension` in this module that declares a forward derivative, a backward derivative or
// a custom forward pass for a function with a reference parameter from another module is
// rejected, whether that module is imported with a plain or an `__exported import`, as for the
// attributes in `primal-elsewhere.slang`. Unlike for a function without a reference
// parameter, an extension that declares a backward derivative reports no error about a plain
// import, because the extension gives the function no derivative. Naming such a function in
// `fwd_diff` or `bwd_diff` then reports the missing derivative, and differentiating a caller of it
// adds no error.

import "cross-module-primal";
__exported import "cross-module-exported-primal";

groupshared float sharedFloat;

struct ApplyContext
{
    float a;
    float x;
    void operator()(__none_t na, out float dx, float dOut)
    {
        dx = dOut * 2.0 * a * x;
    }
}

__func_extension fwd_diff(referenceForwardExtended)(__ref float a, DifferentialPair<float> x) -> DifferentialPair<float>
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

__func_extension bwd_diff(referenceBackwardExtended)(__ref float a, inout DifferentialPair<float> x, float dOut) -> void
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

__func_extension __apply(referenceApplyExtended)(__ref float a, float x) -> Tuple<float, ApplyContext>
//CHECK:                 ^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return makeTuple(a * x * x, ApplyContext(a, x));
}

__func_extension fwd_diff(groupSharedForwardExtended)(groupshared float a, DifferentialPair<float> x) -> DifferentialPair<float>
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

__func_extension bwd_diff(groupSharedBackwardExtended)(groupshared float a, inout DifferentialPair<float> x, float dOut) -> void
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

__func_extension __apply(groupSharedApplyExtended)(groupshared float a, float x) -> Tuple<float, ApplyContext>
//CHECK:                 ^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return makeTuple(a * x * x, ApplyContext(a, x));
}

__func_extension fwd_diff(exportedReferenceForwardExtended)(__ref float a, DifferentialPair<float> x) -> DifferentialPair<float>
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

__func_extension bwd_diff(exportedReferenceBackwardExtended)(__ref float a, inout DifferentialPair<float> x, float dOut) -> void
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

__func_extension __apply(exportedReferenceApplyExtended)(__ref float a, float x) -> Tuple<float, ApplyContext>
//CHECK:                 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return makeTuple(a * x * x, ApplyContext(a, x));
}

__func_extension fwd_diff(exportedGroupSharedForwardExtended)(groupshared float a, DifferentialPair<float> x) -> DifferentialPair<float>
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

__func_extension bwd_diff(exportedGroupSharedBackwardExtended)(groupshared float a, inout DifferentialPair<float> x, float dOut) -> void
//CHECK:                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

__func_extension __apply(exportedGroupSharedApplyExtended)(groupshared float a, float x) -> Tuple<float, ApplyContext>
//CHECK:                 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
{
    return makeTuple(a * x * x, ApplyContext(a, x));
}

[Differentiable]
float callReferenceBackwardExtended(float x)
{
    float a = 3.0;
    return referenceBackwardExtended(a, x);
}

[Differentiable]
float callExportedGroupSharedApplyExtended(float x)
{
    return exportedGroupSharedApplyExtended(sharedFloat, x);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float a = 3.0;
    sharedFloat = 3.0;
    var dp = diffPair(2.0, 0.0);

    outputBuffer[0] = fwd_diff(referenceForwardExtended)(a, diffPair(2.0, 1.0)).d;
/*CHECK:
                      ^^^^^^^^ error E30027
                      ^^^^^^^^ 'fwd_diff' is not a member of 'referenceForwardExtended'.
*/
    bwd_diff(referenceBackwardExtended)(a, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'referenceBackwardExtended'.
*/
    bwd_diff(groupSharedApplyExtended)(sharedFloat, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'groupSharedApplyExtended'.
*/
    outputBuffer[1] = fwd_diff(exportedGroupSharedForwardExtended)(sharedFloat, diffPair(2.0, 1.0)).d;
/*CHECK:
                      ^^^^^^^^ error E30027
                      ^^^^^^^^ 'fwd_diff' is not a member of 'exportedGroupSharedForwardExtended'.
*/
    bwd_diff(exportedGroupSharedBackwardExtended)(sharedFloat, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'exportedGroupSharedBackwardExtended'.
*/
    bwd_diff(exportedReferenceApplyExtended)(a, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'exportedReferenceApplyExtended'.
*/

    outputBuffer[2] = fwd_diff(callReferenceBackwardExtended)(diffPair(2.0, 1.0)).d;
    bwd_diff(callExportedGroupSharedApplyExtended)(dp, 1.0);
}

// The errors below are located in other modules. These annotations come last because an
// annotation without a position matches the first unmatched diagnostic with the same message.
//
//CHECK: 'referenceForwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'referenceBackwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'referenceApplyExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'groupSharedForwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'groupSharedBackwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'groupSharedApplyExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'exportedReferenceForwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'exportedReferenceBackwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'exportedReferenceApplyExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'exportedGroupSharedForwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'exportedGroupSharedBackwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'exportedGroupSharedApplyExtended' cannot be differentiated, because parameter 'a' is passed by reference
