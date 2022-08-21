//TEST(compute):COMPARE_COMPUTE: -shaderobj -output-using-type

struct PlusOne<let v : int>
{ 
    static const int value = v + 1;
}

struct GetConst<let v : int, let u : int>
{
    static const int value = (u+v)*(u+v) + PlusOne<u-1>.value;
    int arr[value];
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;
    int inVal = tid;
    int arr[GetConst<5,2>.value + 1];
    arr[0] = GetConst<5,3>.value + 1;
    outputBuffer[tid] = arr[0];
}
