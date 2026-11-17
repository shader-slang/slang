//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

// A `...DerivativeOf` derivative in this module for a function with a reference parameter from
// another module is rejected, whether that module is imported with a plain or an
// `__exported import`. Unlike for a function without a reference parameter, a backward derivative
// with a plain import reports no error about the import, because the derivative is never attached
// to the function. Naming such a function in `fwd_diff` or `bwd_diff` then reports the missing
// derivative, and differentiating a caller of it adds no error.
// `func-extension-primal-elsewhere.slang` covers the derivatives that a
// `__func_extension` declares.

import "cross-module-primal";
__exported import "cross-module-exported-primal";

groupshared float sharedFloat;

[ForwardDerivativeOf(referenceForwardPrimal)]
/*CHECK:
                     ^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
DifferentialPair<float> referenceForwardDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[BackwardDerivativeOf(referenceBackwardPrimal)]
/*CHECK:
                      ^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
void referenceBackwardDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

[ForwardDerivativeOf(groupSharedForwardPrimal)]
/*CHECK:
                     ^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
DifferentialPair<float> groupSharedForwardDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[BackwardDerivativeOf(groupSharedBackwardPrimal)]
/*CHECK:
                      ^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
void groupSharedBackwardDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

[ForwardDerivativeOf(exportedReferenceForwardPrimal)]
/*CHECK:
                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
DifferentialPair<float> exportedReferenceForwardDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[BackwardDerivativeOf(exportedReferenceBackwardPrimal)]
/*CHECK:
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
void exportedReferenceBackwardDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

[ForwardDerivativeOf(exportedGroupSharedForwardPrimal)]
/*CHECK:
                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
DifferentialPair<float> exportedGroupSharedForwardDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[BackwardDerivativeOf(exportedGroupSharedBackwardPrimal)]
/*CHECK:
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ a user-defined derivative is declared for it here
*/
void exportedGroupSharedBackwardDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

[Differentiable]
float callReferenceBackwardPrimal(float x)
{
    float a = 3.0;
    return referenceBackwardPrimal(a, x);
}

[Differentiable]
float callExportedGroupSharedForwardPrimal(float x)
{
    return exportedGroupSharedForwardPrimal(sharedFloat, x);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float a = 3.0;
    sharedFloat = 3.0;
    var dp = diffPair(2.0, 0.0);

    outputBuffer[0] = fwd_diff(referenceForwardPrimal)(a, diffPair(2.0, 1.0)).d;
/*CHECK:
                      ^^^^^^^^ error E30027
                      ^^^^^^^^ 'fwd_diff' is not a member of 'referenceForwardPrimal'.
*/
    bwd_diff(referenceForwardPrimal)(a, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'referenceForwardPrimal'.
*/
    outputBuffer[1] = fwd_diff(referenceBackwardPrimal)(a, diffPair(2.0, 1.0)).d;
/*CHECK:
                      ^^^^^^^^ error E30027
                      ^^^^^^^^ 'fwd_diff' is not a member of 'referenceBackwardPrimal'.
*/
    bwd_diff(referenceBackwardPrimal)(a, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'referenceBackwardPrimal'.
*/
    outputBuffer[2] = fwd_diff(exportedGroupSharedForwardPrimal)(sharedFloat, diffPair(2.0, 1.0)).d;
/*CHECK:
                      ^^^^^^^^ error E30027
                      ^^^^^^^^ 'fwd_diff' is not a member of 'exportedGroupSharedForwardPrimal'.
*/
    bwd_diff(exportedGroupSharedForwardPrimal)(sharedFloat, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'exportedGroupSharedForwardPrimal'.
*/
    outputBuffer[3] = fwd_diff(exportedGroupSharedBackwardPrimal)(sharedFloat, diffPair(2.0, 1.0)).d;
/*CHECK:
                      ^^^^^^^^ error E30027
                      ^^^^^^^^ 'fwd_diff' is not a member of 'exportedGroupSharedBackwardPrimal'.
*/
    bwd_diff(exportedGroupSharedBackwardPrimal)(sharedFloat, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'exportedGroupSharedBackwardPrimal'.
*/

    outputBuffer[4] = fwd_diff(callReferenceBackwardPrimal)(diffPair(2.0, 1.0)).d;
    bwd_diff(callExportedGroupSharedForwardPrimal)(dp, 1.0);
}

// The errors and notes below are located in other modules. These annotations come last because
// an annotation without a position matches the first unmatched diagnostic with the same message.
//
//CHECK: 'referenceForwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'referenceBackwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'groupSharedForwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'groupSharedBackwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'exportedReferenceForwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'exportedReferenceBackwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'exportedGroupSharedForwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: 'exportedGroupSharedBackwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
