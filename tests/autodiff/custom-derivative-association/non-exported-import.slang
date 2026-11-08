//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

// A `[BackwardDerivativeOf]` derivative is rejected when the function it differentiates comes from
// a module imported without `__exported import`. A `[ForwardDerivativeOf]` derivative is not, as
// `cross-module.slang` shows. The error is reported twice at the attribute.
// `func-extension-non-exported-import.slang` covers the same rule for a `__func_extension`.

import "cross-module-primal";

[BackwardDerivativeOf(nonExportedBackwardPrimal)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^^ error E31162
 ^^^^^^^^^^^^^^^^^^^^ a publicly visible custom derivative cannot reference 'nonExportedBackwardPrimal' from a module that is imported but not re-exported with '__exported import'.
 ^^^^^^^^^^^^^^^^^^^^ error E31162
 ^^^^^^^^^^^^^^^^^^^^ a publicly visible custom derivative cannot reference 'nonExportedBackwardPrimal' from a module that is imported but not re-exported with '__exported import'.
*/
void nonExportedBackwardDerivative(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var dp = diffPair(2.0, 0.0);
    bwd_diff(nonExportedBackwardPrimal)(3.0, dp, 1.0);
    outputBuffer[0] = dp.d;
}

// The error is also reported at the core-module declaration of the requirement that the
// derivative satisfies. These annotations come last because an annotation without a position
// matches the first unmatched diagnostic with the same message.
//
//CHECK: custom derivative uses non-exported import
//CHECK: a publicly visible custom derivative cannot reference 'nonExportedBackwardPrimal' from a module that is imported but not re-exported with '__exported import'.
