//TEST:SIMPLE(filecheck=SPV): -target spirv -O0

// Test that we can defer buffer loads by ruling out potential aliasing writes.

struct Bottom
{
    float bigArray[1024];

    float bottomGetValue(int index)
    {
        // RWStructuredBuffer is considered to not alias with anything else.
        // this write should not prevent deferring loading bigArray.
        gOther[0] = 100;
        // this write should not prevent deferring loading bigArray.
        gSharedVar = 1;
        // this write should not prevent deferring loading bigArray.
        gStaticVar = 2;

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
static int gStaticVar;
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
