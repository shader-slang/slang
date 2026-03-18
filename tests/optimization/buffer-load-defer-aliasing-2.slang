//TEST:SIMPLE(filecheck=SPV): -target spirv -O0

// Test that we can defer buffer loads across multiple basic blocks.

struct Bottom
{
    float bigArray[1024];

    float bottomGetValue(int index)
    {
        gSharedVar = 10;

        for (int i = 0; i < gUniformVar; ++i)
        {
            // RWStructuredBuffer is considered to not alias with anything else.
            // this write should not prevent deferring loading bigArray.
            gOther[i] = i;
        }

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
uniform RWStructuredBuffer<int> gOther;
uniform int gUniformVar;
groupshared int gSharedVar;

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    // SPV: OpEntryPoint
    // SPV-NOT: OpLoad %Bottom_natural
    outputBuffer[0] = gRoot.bottom1.bottomGetValue(0);
}
