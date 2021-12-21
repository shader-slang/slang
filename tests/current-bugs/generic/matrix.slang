//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj

/* A test to use generics around resource/built in types. 

Doesn't work

.slang(5): error 30019: expected an expression of type 'int', got 'typeof(W)'
    matrix<float, W, H> values;
 */

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Another<W : int, H : int>
{
    matrix<float, W, H> values;
};

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x;

    Another<2, 4> a = {};
   
	outputBuffer[index] = index;
}

