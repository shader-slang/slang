//TEST(compute):COMPARE_COMPUTE: -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE: -vk -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static int x = 0;

func incX() -> float4
{
    x += 10;
    return {0,0,0,0};
}

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;

    false ? x++ : x++;

    bool4(false) ? incX() : incX();

    bool a = true || false;
    if(a)
    outputBuffer[tid] = x;
}
