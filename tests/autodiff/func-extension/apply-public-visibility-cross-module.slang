//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature

import apply_public_visibility_cross_module_helper;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var input = diffPair(2.0, 0.0);
    bwd_diff(identityWithCustomGradient<float>)(input, 1.0);
    outputBuffer[0] = input.d;
    // CHECK: 7.0
}
