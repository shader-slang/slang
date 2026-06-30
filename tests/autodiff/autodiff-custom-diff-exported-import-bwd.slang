//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Verifies that a public custom backward derivative remains usable through an
// `__exported import` chain across three modules.

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import "autodiff-custom-diff-exported-import-lib";

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var dx = diffPair(2.0);
    bwd_diff(exportedCube)(dx, 1.0);
    outputBuffer[0] = dx.d;

    var dy = diffPair(3.0);
    helper_bwd(dy, 1.0);
    outputBuffer[1] = dy.d;

    // CHECK:      14.0
    // CHECK-NEXT: 21.0
}
