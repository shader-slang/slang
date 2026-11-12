//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

// A module that declares a `...DerivativeOf` derivative for a function with a reference parameter
// from another module is rejected, whether it imports that module with a plain or an
// `__exported import`, and importing it then fails. The error is located at the function's
// reference parameter in the module that declares the function, and its note at the attribute.
// `func-extension-derivative-library.slang` covers the derivatives that a
// `__func_extension` declares.

import "cross-module-derivative";
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^^^ error E39999
       ^^^^^^^^^^^^^^^^^^^^^^^^^ import of module 'cross-module-derivative' failed because of a compilation error
*/

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0.0;
}

// The errors and notes below are located in other modules, and the final "compilation ceased" has
// no location. These annotations come last because an annotation without a position matches the
// first unmatched diagnostic with the same message.
//
//CHECK: 'referenceForwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'referenceBackwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'groupSharedForwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'groupSharedBackwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'exportedReferenceForwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'exportedGroupSharedBackwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'exportedReferenceBackwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'exportedGroupSharedForwardPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: compilation ceased
