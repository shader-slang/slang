//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// A library module that declares public backward derivatives. A `[BackwardDerivativeOf]`
// derivative for a function from a module imported without `__exported import` is rejected, as
// in `non-exported-import.slang`.

module "non-exported-import-library";

import "cross-module-primal";

[BackwardDerivativeOf(nonExportedLibraryBackwardPrimal)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^^ error E31162
 ^^^^^^^^^^^^^^^^^^^^ a publicly visible custom derivative cannot reference 'nonExportedLibraryBackwardPrimal' from a module that is imported but not re-exported with '__exported import'.
 ^^^^^^^^^^^^^^^^^^^^ error E31162
 ^^^^^^^^^^^^^^^^^^^^ a publicly visible custom derivative cannot reference 'nonExportedLibraryBackwardPrimal' from a module that is imported but not re-exported with '__exported import'.
*/
public void nonExportedLibraryBackwardDerivative(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

// The error is also reported at the core-module declaration of the requirement that the
// derivative satisfies. These annotations come last because an annotation without a position
// matches the first unmatched diagnostic with the same message.
//
//CHECK: custom derivative uses non-exported import
//CHECK: a publicly visible custom derivative cannot reference 'nonExportedLibraryBackwardPrimal' from a module that is imported but not re-exported with '__exported import'.
