//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK): -output-using-type

// Test that generic argument inference works when passing a scalar to a generic vector parameter.
T process<T, let N : int>(vector<T, N> v)
{
    return v[0];
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    float a = 1.0;
    outputBuffer[0] = process(a);
    // CHECK: 1.0
}
