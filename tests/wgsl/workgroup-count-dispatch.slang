// WorkgroupCount() at run time: a 2 x 3 x 1 dispatch of one-thread groups, and every group reads
// 2, 3, 1.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -shaderobj -wgpu -output-using-type -compute-dispatch 2,3,1
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -shaderobj -vk -output-using-type -compute-dispatch 2,3,1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// BUF-COUNT-6: 231
[numthreads(1, 1, 1)]
void computeMain(uint3 gid: SV_GroupID)
{
    uint3 groups = WorkgroupCount();
    outputBuffer[gid.y * groups.x + gid.x] = groups.x * 100 + groups.y * 10 + groups.z;
}
