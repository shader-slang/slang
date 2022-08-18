//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj 

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

/* Tests purpose is to confirm that use of `= {}` initialization 
works with a generic */

struct Check<T>
{
    T v;
};

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);

    Check<float> v = {};
    
    outputBuffer[index] = index + int(v.v); 
}

