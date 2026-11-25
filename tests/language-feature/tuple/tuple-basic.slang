// Test tuple swizzling and element access.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void store(inout Tuple<int, float, uint> t)
{
    t._1_2 = Tuple<float, uint>(3.0, 4);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var t = Tuple<int, float, uint>(1, 2.0, 3u);
    store(t);
    let y = t._1_2_0;

    // CHECK: 4
    outputBuffer[0] = y._1;
}
