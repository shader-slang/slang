//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

// A backward-derivative extension is rejected when the function it extends comes from a module
// imported without `__exported import`, as a `[BackwardDerivativeOf]` attribute is in
// `non-exported-import.slang`, and the error is reported twice at the extension. A
// forward-derivative extension and a custom forward pass are not rejected, as
// `func-extension-cross-module.slang` shows, even though a custom forward pass also gives the
// function a backward derivative.

import "cross-module-primal";

__func_extension bwd_diff(nonExportedBackwardExtended)(no_diff float a, inout DifferentialPair<float> x, float dOut) -> void
//CHECK:         ^^^^^^^^ error E31162
//CHECK:         ^^^^^^^^ a publicly visible custom derivative cannot reference 'nonExportedBackwardExtended' from a module that is imported but not re-exported with '__exported import'.
//CHECK:         ^^^^^^^^ error E31162
//CHECK:         ^^^^^^^^ a publicly visible custom derivative cannot reference 'nonExportedBackwardExtended' from a module that is imported but not re-exported with '__exported import'.
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var dp = diffPair(2.0, 0.0);
    bwd_diff(nonExportedBackwardExtended)(3.0, dp, 1.0);
    outputBuffer[0] = dp.d;
}

// The error is also reported at the core-module declaration of the requirement that the
// derivative satisfies. These annotations come last because an annotation without a position
// matches the first unmatched diagnostic with the same message.
//
//CHECK: custom derivative uses non-exported import
//CHECK: a publicly visible custom derivative cannot reference 'nonExportedBackwardExtended' from a module that is imported but not re-exported with '__exported import'.
