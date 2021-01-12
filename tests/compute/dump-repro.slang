//DISABLE_TEST(compute):SIMPLE:-profile cs_5_0 -entry computeMain -target dxbc -dump-repro repro.slang-repro
//TEST:COMPILE:tests/compute/dump-repro.slang -profile cs_5_0 -entry computeMain -target dxbc -dump-repro repro.slang-repro
//TEST(compute):COMPARE_COMPUTE_EX:-dx11 -compute -no-default-entry-point -compile-arg -load-repro -compile-arg repro.slang-repro -shaderobj
// We only want to run on one API to test load worked
//DISABLE_TEST(compute):COMPARE_COMPUTE:-dx12 -load-repro repro.slang-repro -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    
	outputBuffer[index] = index * 2 + 1;
}
