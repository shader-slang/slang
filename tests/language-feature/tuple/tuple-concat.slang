// Test tuple swizzling and element access.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct GetCount<let N : int>
{
    static const int value = N;
}

int getCount<each T>(Tuple<T> t)
{
    return GetCount<countof(T)>.value;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var t = makeTuple(1, 2.0);
    var u = makeTuple(3u, 4.0);
    var r = concat(t, u);

    // CHECK: 4
    outputBuffer[0] = (int)r._3_2._0;

    // CHECK: 2
    outputBuffer[1] = (int)r._1;

    // CHECK: 8
    outputBuffer[2] = countof(r) + getCount(r);
}
