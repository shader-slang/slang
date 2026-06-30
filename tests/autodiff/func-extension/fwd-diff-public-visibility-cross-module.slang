//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature

// TODO: re-enable when func-extension visibility is implemented separately.

// Test: a public func-extension is accessible from another module.

import fwd_diff_visibility_cross_module_helper;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = fwd_diff(cubePublic)(diffPair(2.0, 1.0)).d;
    // CHECK: 12.0
}
