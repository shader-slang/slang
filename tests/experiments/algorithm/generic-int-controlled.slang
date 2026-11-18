//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int reduce<let Clustering : int, let DupTest : int = 1>( int a)
{
    if (Clustering != 0)
    {
        a = a + a;
    }
    if (DupTest != 0)
    {
        a *= a;
    }
    return a;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x;

    int r = reduce<1>(index);

    // CHECK: 0
    // CHECK: 4
    // CHECK: 10
    // CHECK: 24
	outputBuffer[index] = r;
}
