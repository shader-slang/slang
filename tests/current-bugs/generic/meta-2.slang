//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj

/* A test meta programing.

Doesn't work:

.slang(7): error 30019: expected an expression of type 'int', got 'typeof(N)'
    static const int Value = N;
 */

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct GetValue<N : int>
{
    static const int Value = N;
};
  
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int index = dispatchThreadID.x;

	outputBuffer[index] = GetValue<10>::Value;
}

