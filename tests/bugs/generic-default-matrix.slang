//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Another<let W : int, let H : int>
{
    matrix<float, W, H> values;
};

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x;

    Another<2, 4> a = {};
   
	outputBuffer[index] = index + a.values[0].x;
}

