// Test for limited support of groupshared pointers.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type -emit-spirv-directly

groupshared int gData[1024];

void writeData(Ptr<int, Access.ReadWrite, AddressSpace.GroupShared> pData, int v)
{
    *pData = v;
}

Ptr<int, Access.ReadWrite, AddressSpace.GroupShared> getPtr(int offset)
{
    return spirv_asm
    {
        result:$$Ptr<int, Access.ReadWrite, AddressSpace.GroupShared> = OpInBoundsAccessChain &gData $offset
    };
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride = 4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(2,1,1)]
void computeMain(int tid : SV_GroupThreadID)
{
    let p = getPtr(tid);
    writeData(p, (tid+1) * 100);
    GroupMemoryBarrierWithGroupSync();
    if (tid == 0)
    {
        // CHECK: 300 
        outputBuffer[0] = gData[0] + gData[1];
    }
}