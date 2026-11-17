//TEST:SIMPLE(filecheck=SPV): -target spirv

// Test that a write into a bound resource conservatively prevents deferring a
// load through a user pointer: a user pointer can refer to the contents of a
// bound resource (for example `__getAddress(buf[i])`), so deferring the load
// past the write would read the post-write contents, while the program must
// observe the pre-write snapshot of `this`. The whole-struct load stays.

struct Bottom
{
    float bigArray[1024];

    float bottomGetValue(int index)
    {
        // This write may reach the memory `gRoot` points to, so it must
        // prevent deferring loading bigArray.
        gOther[0] = 100;

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

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    // SPV: OpEntryPoint
    // SPV: OpLoad %Bottom_natural
    outputBuffer[0] = gRoot.bottom1.bottomGetValue(0);
}
