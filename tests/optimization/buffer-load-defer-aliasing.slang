//TEST:SIMPLE(filecheck=SPV): -target spirv -O0

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
    // SPV: OpLoad %Bottom_natural
    outputBuffer[0] = gRoot.bottom1.bottomGetValue(0);
}
