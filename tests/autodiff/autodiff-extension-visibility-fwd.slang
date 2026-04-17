//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import "autodiff-extension-visibility-lib";

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = VisibilityCase.publicSquare.fwd_diff(diffPair(3.0, 1.0)).d;
    outputBuffer[1] = VisibilityCase.callPublicFwd(3.0);
    outputBuffer[2] = VisibilityCase.callInternalFwd(2.0);
    outputBuffer[3] = VisibilityCase.callPrivateFwd(2.0);
    outputBuffer[4] = VisibilityCase.callPrivateCustomFwd(3.0);

    // CHECK:      6.0
    // CHECK-NEXT: 6.0
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 32.0
    // CHECK-NEXT: 15.0
}
