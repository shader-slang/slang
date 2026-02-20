//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out, name outputBuffer
RWStructuredBuffer<float> outputBuffer;

typealias SharedFloatPtr = Ptr<float, Access::ReadWrite, AddressSpace::GroupShared>;

struct MyDiffPtr
{
    SharedFloatPtr primalPtr;
    SharedFloatPtr diffPtr;

    [BackwardDerivative(load_bwd)]
    float load()
    {
        return primalPtr[0];
    }

    void load_bwd(float dOut)
    {
        if (diffPtr != nullptr)
        {
            diffPtr[0] += dOut;
        }
    }
};

[Differentiable]
float function(MyDiffPtr diffPtr)
{
    return diffPtr.load() * diffPtr.load() * 2.0f;
}

groupshared float sbuf[2];

[numthreads(1, 1, 1), shader("compute")]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    sbuf[0] = 3.0f;
    sbuf[1] = 0.0f;
    
    bwd_diff(function)({ __getAddress(sbuf[0]), __getAddress(sbuf[1]) }, 1.f);

    outputBuffer[0] = sbuf[1];
    // CHECK: 12.0
}