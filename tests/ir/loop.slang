//TEST:SIMPLE:-use-ir -dump-ir -skip-codegen -target dxbc -profile cs_5_0 -entry main

#define GROUP_THREAD_COUNT 64

StructuredBuffer<float4> input;
RWStructuredBuffer<float4> output;

groupshared float4 s[GROUP_THREAD_COUNT];

[numthreads(GROUP_THREAD_COUNT, 1, 1)]
void main(
    uint dispatchThreadID   : SV_DispatchThreadID,
    uint groupThreadID      : SV_GroupThreadID,
    uint groupID            : SV_GroupIndex )
{
    // the actual algorithm being done here is bogus

    // load shared memory
    s[groupThreadID] = input[dispatchThreadID];

    // do some sum passes
    for(uint stride = 1; stride < GROUP_THREAD_COUNT; stride <<= 1)
    {
        GroupMemoryBarrierWithGroupSync();

        s[groupThreadID] += s[groupThreadID - stride];
    }

    GroupMemoryBarrierWithGroupSync();

    output[dispatchThreadID] = s[0];
}

