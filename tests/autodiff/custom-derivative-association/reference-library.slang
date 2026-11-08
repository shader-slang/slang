//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

// A module that requests derivatives of functions with reference parameters is rejected in the
// same way when another module imports it as when it is compiled on its own, and the import then
// fails. Each request is rejected at the function's reference parameter. The note for a
// differentiability or derivative attribute says that it requests a derivative, and the note for
// a `...DerivativeOf` attribute says that it declares one. `func-extension-reference-library.slang`
// covers the derivatives that a `__func_extension` declares.

import "cross-module-reference-library";
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error E39999
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ import of module 'cross-module-reference-library' failed because of a compilation error
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
//CHECK: 'referenceSynthesized' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: this requests a derivative; remove it if the function needs none
//CHECK: 'groupSharedSynthesized' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: this requests a derivative; remove it if the function needs none
//CHECK: 'referenceForwardNamed' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: this requests a derivative; remove it if the function needs none
//CHECK: 'groupSharedForwardNamed' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: this requests a derivative; remove it if the function needs none
//CHECK: 'referenceBackwardNamed' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: this requests a derivative; remove it if the function needs none
//CHECK: 'groupSharedBackwardNamed' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: this requests a derivative; remove it if the function needs none
//CHECK: 'referenceForwardOfPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'groupSharedForwardOfPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'referenceBackwardOfPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: 'groupSharedBackwardOfPrimal' cannot be differentiated, because parameter 'a' is passed by reference
//CHECK: a user-defined derivative is declared for it here
//CHECK: compilation ceased
