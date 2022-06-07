// bound-check-zero-index.slang

// Check 'zero indexing' bound check feature, supported by CPU and CUDA

//DISABLE_TEST(compute):COMPARE_COMPUTE:-cuda -shaderobj -Xslang... -Xnvrtc... -DSLANG_ENABLE_BOUND_ZERO_INDEX -X. -X.
//DISABLE_TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj -Xslang... -Xllvm... -DSLANG_ENABLE_BOUND_ZERO_INDEX -X. -X.

//TEST_INPUT:ubuffer(data=[0 1 2 3 ]):name=byteAddressBuffer
ByteAddressBuffer byteAddressBuffer;

//TEST_INPUT:ubuffer(data=[4 5 6 7]):name=rwByteAddressBuffer
RWByteAddressBuffer rwByteAddressBuffer;

//TEST_INPUT:ubuffer(data=[10 20 30 40], stride=4):name=structuredBuffer
StructuredBuffer<int> structuredBuffer;

//TEST_INPUT:ubuffer(data=[-500 -100 -200 -300], stride=4):name=rwStructuredBuffer
RWStructuredBuffer<int> rwStructuredBuffer;

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer2
RWStructuredBuffer<int> outputBuffer2;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	int tid = dispatchThreadID.x;
    
    int fixedArray[3] = { 2, 5, 9};
    
    int total = 0;
    total += byteAddressBuffer.Load<int>(tid * 4);
    total += byteAddressBuffer.Load<int>(-tid * 4);
    
    total += rwByteAddressBuffer.Load<int>(tid * 4);
    total += rwByteAddressBuffer.Load<int>(-tid * 4);
    
    total += structuredBuffer[tid];
    total += structuredBuffer[-tid];
    
    total += rwStructuredBuffer[tid];
    total += rwStructuredBuffer[-tid];
    
    outputBuffer[-tid] = total;
    outputBuffer2[tid * 2 + 1] = total;
}