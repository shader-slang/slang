// This test is based on issue #9007 where padding to alignment in entry point parameter structs
// causes TypeLayoutReflection::getSize() to return a padding size that, if used for sizing a parameter
// buffer, causes cuLaunchKernel to fail
// This test should continue to pass after the workaround in slang-rhi is removed

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=input
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set count = 4

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform StructuredBuffer<float> input,
    uniform RWStructuredBuffer<float> output,
    uniform uint count)
{
    if (tid >= count)
        return;

    // CHECK: 1.0
    // CHECK: 2.0
    // CHECK: 3.0
    // CHECK: 4.0
    output[tid] = input[tid];
}
