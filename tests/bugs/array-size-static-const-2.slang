// array-size-static-const-2.hlsl
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct GenType<let N : int>
{
    static const int HalfN = N / 2;
    static int sum(int arr[HalfN])
    {
        int rs = 0;
        for (int i = 0; i < HalfN; i++)
            rs += arr[i];
        return rs;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    int arr[2] = { 1, 2 };
    outputBuffer[0] = GenType<4>.sum(arr);
}