//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

// A module that declares a `__func_extension` derivative for a function with a reference
// parameter from another module is rejected, whether it imports that module with a plain or an
// `__exported import`, and importing it then fails, as for the attributes in
// `derivative-library.slang`. The error is located at the function's reference
// parameter in the module that declares the function, and its note at the extension.

import "cross-module-extension-derivative";
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error E39999
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ import of module 'cross-module-extension-derivative' failed because of a compilation error
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
//CHECK: 'referenceForwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'groupSharedBackwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'referenceApplyExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'exportedReferenceApplyExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'exportedGroupSharedBackwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: compilation ceased
