//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

// An extension cannot be `public`, so the forward derivative that a `__func_extension` in another
// module declares is not visible here, even when that module also declares the function. Naming
// the function in `fwd_diff` finds the extension's member but cannot access it, and naming it in
// `bwd_diff` finds no member, as for any function with only a forward derivative.

import "cross-module-extension-library";

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = fwd_diff(libraryForwardExtended)(3.0, diffPair(2.0, 1.0)).d;
//CHECK:              ^^^^^^^^ error E30600
//CHECK:              ^^^^^^^^ 'fwd_diff' is not accessible from the current context.

    var dp = diffPair(2.0, 0.0);
    bwd_diff(libraryForwardExtended)(3.0, dp, 1.0);
/*CHECK:
    ^^^^^^^^ error E30027
    ^^^^^^^^ 'bwd_diff' is not a member of 'libraryForwardExtended'.
*/
}
