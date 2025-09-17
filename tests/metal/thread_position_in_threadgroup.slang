//TEST:SIMPLE(filecheck=METAL): -entry computeMain -stage compute -target metal -D GROUPID
//TEST:SIMPLE(filecheck=METAL): -entry computeMain -stage compute -target metal
//DISABLE_TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -output-using-type
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -output-using-type

// Compute test disabled; Buffer isn't handled by the test infrastructure
// TODO: github issue #8456

//METAL: computeMain

// ensure we compute the SV_GroupIndex from SV_GroupThreadID and `numthreads`
// METAL: thread_position_in_threadgroup
// METAL-DAG: *{{.*}}2

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name dst
RWBuffer<uint> dst;

void indirection(uint groupIndex)
{
    dst[groupIndex] = groupIndex;
}

#define THREAD_COUNT 2
[numthreads(THREAD_COUNT, 1, 1)]
#ifdef GROUPID
void computeMain(uint GI : SV_GroupIndex, uint GTID : SV_GroupThreadID)
#else
void computeMain(uint GI : SV_GroupIndex)
#endif
{
    // BUF: 0
    // BUF: 1
    dst[GI + THREAD_COUNT] = GI;
    // BUF: 0
    // BUF: 1
    indirection(GI);
} 
