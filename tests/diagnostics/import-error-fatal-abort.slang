//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry fragMain -stage fragment

// An error inside an imported module must surface as an ordinary diagnostic in the importing
// module, followed by a fatal "compilation ceased" — never as an out-of-band process kill.
// A build with SLANG_DISABLE_EXCEPTIONS used to terminate the process here instead of
// returning (shader-slang/slang#14474); its recovery path needs such a build to execute.
//CHECK:   ^^^^^^^^^^^^^^^^^^^ undefined identifier 'someUndefinedSymbol'.
// The caret annotation above matches by (line, column) only — the framework does not compare
// file names — so it pins the imported module's undefined-identifier diagnostic, which sits at
// the same line number (6) in import-error-fatal-abort-broken.slang as the comment line the
// annotation follows here; the trailing //CHECK lines pin the position-less diagnostics.

import "import-error-fatal-abort-broken";
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error E39999
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ import of module 'import-error-fatal-abort-broken' failed because of a compilation error
*/

[shader("fragment")]
float4 fragMain() : SV_Target
{
    return brokenHelper();
}

// The undefined-identifier diagnostic is located in the imported module, and the final
// "compilation ceased" has no location. These annotations come last because an annotation
// without a position matches the first unmatched diagnostic with the same message.
//
//CHECK: undefined identifier
//CHECK: compilation ceased
