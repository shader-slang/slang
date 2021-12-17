//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj

/* A test around use of an array like container. 

Doesn't work because SIZE can't be used to set the element array size */

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct FixedArray<T, SIZE : int> 
{
    
    T elements[SIZE];
};

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x;

    FixedArray<int, 8> arr;
    
	outputBuffer[index] = 1;
}

