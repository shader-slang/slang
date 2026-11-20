//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type
//TEST:COMPARE_COMPUTE(targets=metal, filecheck-buffer=CHECK):-output-using-type -render-features argument-buffer-tier-2

struct TestStruct<Format:__BuiltinIntegerType, let count : int>
{
    Format f;
};

Format testFunction<Format : __BuiltinIntegerType, let count : int>(TestStruct<Format, count> data)
{
    return data.f + __int_cast<Format>(count);
}

//TEST_INPUT: set testBlock = new TestStruct<int, 12>{1}
ParameterBlock<TestStruct<int, 12>> testBlock;

//TEST_INPUT: set testBlock2 = new TestStruct<int, 12>{2}
ConstantBuffer<TestStruct<int, 2>> testBlock2;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: 13
    outputBuffer[0] = testFunction(testBlock);
    // CHECK: 13
    outputBuffer[1] = testFunction<int, 12>(testBlock);
    // CHECK: 4
    outputBuffer[2] = testFunction(testBlock2);
}