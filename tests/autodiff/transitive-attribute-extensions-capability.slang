// A caller that imports only the facade differentiates `ExposedOperations.spirvBackwardPrimal`
// and `ExposedOperations.spirvForwardPrimal`. Their derivatives are attached in the library with
// `[BackwardDerivativeOf]` / `[ForwardDerivativeOf]` and carry `[require(spirv)]`, so
// differentiating them on `hlsl` fails with E36107, exactly as it does with a direct import.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKBWD,non-exhaustive): -target hlsl -entry computeMainDiffBwd -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKFWD,non-exhaustive): -target hlsl -entry computeMainDiffFwd -stage compute

// The same derivatives are usable on a target that provides the capability.
//TEST:SIMPLE(filecheck=CHECKSPIRV): -target spirv -entry computeMainDiffBwd -stage compute

import "transitive-attribute-extensions-facade";

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainDiffBwd(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECKBWD:
     ^^^^^^^^^^^^^^^^^^ error E36107
*/
{
    //CHECKSPIRV-NOT: 36107
    //CHECKSPIRV: OpEntryPoint
    var x = diffPair(2.0, 0.0);
    bwd_diff(ExposedOperations.spirvBackwardPrimal)(x, 1.0);
    outputBuffer[0] = x.d;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainDiffFwd(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECKFWD:
     ^^^^^^^^^^^^^^^^^^ error E36107
*/
{
    outputBuffer[0] = fwd_diff(ExposedOperations.spirvForwardPrimal)(diffPair(2.0, 1.0)).d;
}
