//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -output-using-type

// This test just ensures that we compile and run the code.
// It does not check the correctness of the autodiff.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out, name outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct MyDiffPtr
{
    float data1;
    float data2;
};

[Differentiable]
float function(Ptr<MyDiffPtr, Access::ReadWrite, AddressSpace::GroupShared> i)
{
    return i[0].data1 + i[1].data2;
}

groupshared MyDiffPtr s[2];
[numthreads(1, 1, 1), shader("compute")]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    s = { { 0, 2 }, { 1, 3 } };
    float result = 1.0f;
    let pair = __fwd_diff(function)(__getAddress(s[0]));
    outputBuffer[0] = pair.getPrimal();
    outputBuffer[1] = pair.getDifferential();
    // CHECK: 3.0
    // CHECK-NEXT: 0.0
}