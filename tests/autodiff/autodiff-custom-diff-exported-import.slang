//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Verifies that a public custom derivative remains usable through an
// `__exported import` chain across three modules.

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import "autodiff-custom-diff-exported-import-lib";

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = fwd_diff(exportedSquare)(diffPair(3.0, 1.0)).d;
    outputBuffer[1] = helper_fwd(diffPair(4.0, 1.0)).d;

    // CHECK:      15.0
    // CHECK-NEXT: 20.0
}
