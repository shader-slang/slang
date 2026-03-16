//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct CountHolder<let N : int>
{
    static int get() { return N; }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 2
    outputBuffer[0] = CountHolder<countof(__trimHead(Tuple<int, float, bool>))>.get();

    // CHECK: 0
    outputBuffer[1] = CountHolder<countof(__trimHead(Tuple<int>))>.get();
}
