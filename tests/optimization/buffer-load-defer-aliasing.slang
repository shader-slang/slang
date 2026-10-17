//TEST:SIMPLE(filecheck=SPV): -target spirv

// Test that we are not deferring buffer loads due to potential aliasing writes.

struct Bottom
{
    float bigArray[1024];

    float bottomGetValue(int index)
    {
        // this write may cause data stored at gRoot to be modified,
        // thus bigArray[index] may be different from what it was before the call to
        // bottomGetValue. So we should not defer loading bigArray until after this write.
        *gOther = 100; 

        // We should return the value from bigArray from a previously loaded value of `this`.
        return bigArray[index];
    }
}

struct Root
{
    Bottom bottom1;
    Bottom bottom2;
}

uniform Root* gRoot;
uniform int* gOther;

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    // The value must be loaded before the aliasing store to `*gOther`, and not again after it.
    // SPV-LABEL: %Bottom_bottomGetValue = OpFunction
    // SPV: %[[P:[0-9]+]] = OpAccessChain %_ptr_PhysicalStorageBuffer_float
    // SPV: %[[V:[0-9]+]] = OpLoad %float %[[P]]
    // SPV: OpStore %{{[0-9]+}} %int_100
    // SPV-NOT: OpLoad %float
    // SPV: OpReturnValue %[[V]]
    outputBuffer[0] = gRoot.bottom1.bottomGetValue(0);
}
