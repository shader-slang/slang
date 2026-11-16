//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry fragMain -stage fragment

// An error inside an imported module must surface as an ordinary diagnostic in the importing
// module, followed by a fatal "compilation ceased" — as diagnostics, never as an out-of-band
// process kill. In a build with SLANG_DISABLE_EXCEPTIONS the fatal diagnostic used to trap and
// terminate the embedding process (shader-slang/slang#14474); this pins the recovered behavior.
//CHECK:   ^^^^^^^^^^^^^^^^^^^ undefined identifier 'someUndefinedSymbol'.

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
