//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import "autodiff-extension-visibility-lib";

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = VisibilityCase.callPublicBwd(2.0);
    outputBuffer[1] = VisibilityCase.callInternalBwd(2.0);
    outputBuffer[2] = VisibilityCase.callPrivateBwd(2.0);
    outputBuffer[3] = VisibilityCase.callPrivateCustomBwd(3.0);

    // CHECK:      12.0
    // CHECK-NEXT: 32.0
    // CHECK-NEXT: 80.0
    // CHECK-NEXT: 21.0
}
