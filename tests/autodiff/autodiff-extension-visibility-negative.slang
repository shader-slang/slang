//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

import "autodiff-extension-visibility-lib";

void test()
{
    fwd_diff(VisibilityCase.internalCube)(diffPair(2.0, 1.0));
    //CHECK: declaration not accessible
    //CHECK: 'internalCube' is not accessible from the current context.

    fwd_diff(VisibilityCase.privatePow4)(diffPair(2.0, 1.0));
    //CHECK: declaration not accessible
    //CHECK: 'privatePow4' is not accessible from the current context.

    fwd_diff(VisibilityCase.publicCustomSquareWithInternalDerivative)(diffPair(2.0, 1.0));
    //CHECK: declaration not accessible
    //CHECK: 'fwd_diff' is not accessible from the current context.

    var dx = diffPair(2.0);
    bwd_diff(VisibilityCase.internalPow4)(dx, 1.0);
    //CHECK: declaration not accessible
    //CHECK: 'internalPow4' is not accessible from the current context.

    var dy = diffPair(2.0);
    bwd_diff(VisibilityCase.privatePow5)(dy, 1.0);
    //CHECK: declaration not accessible
    //CHECK: 'privatePow5' is not accessible from the current context.
}
