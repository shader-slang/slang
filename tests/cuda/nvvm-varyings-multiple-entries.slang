//TEST:SIMPLE(filecheck=THREAD):-target ptx -emit-cuda-via-nvvm -entry threadMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=GROUP):-target ptx -emit-cuda-via-nvvm -entry groupMain -stage compute -capability cuda_sm_8_0 -O0 -o -

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(3, 2, 2)]
void threadMain(uint3 threadID : SV_GroupThreadID)
{
    outputBuffer[threadID.x + 3 * threadID.y + 6 * threadID.z] =
        all(threadID == cudaThreadIdx()) ? 1 : 0;
}

[shader("compute")]
[numthreads(2, 3, 2)]
void groupMain(uint3 groupID : SV_GroupID)
{
    outputBuffer[groupID.x + 2 * groupID.y + 6 * groupID.z] =
        all(groupID == cudaBlockIdx()) ? 1 : 0;
}

// THREAD: .visible .entry threadMain
// THREAD: %tid.x
// GROUP: .visible .entry groupMain
// GROUP: %ctaid.x
