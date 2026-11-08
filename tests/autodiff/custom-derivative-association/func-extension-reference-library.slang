//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

// A module that declares `__func_extension` derivatives for functions with reference parameters
// is rejected in the same way when another module imports it as when it is compiled on its own,
// and the import then fails, as for the attributes in `reference-library.slang`. Each extension
// is rejected at the function's reference parameter, with a note that the extension declares a
// derivative.

import "cross-module-reference-extension-library";
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error E39999
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ import of module 'cross-module-reference-extension-library' failed because of a compilation error
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
//CHECK: 'groupSharedForwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'referenceBackwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'groupSharedBackwardExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'referenceApplyExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'groupSharedApplyExtended' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: compilation ceased
