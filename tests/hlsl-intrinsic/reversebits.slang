//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    bool result = true;
    result = result && reversebits(0u) == 0u;
    result = result && reversebits(-1) == -1;
    result = result && reversebits(0x50505050u) == 0x0A0A0A0Au;
    result = result && reversebits(1u) == (1u<<31u);
    result = result && reversebits(1u<<31u) == 1u;

    outputBuffer[0] = (result == true) ? 1 : 0;
}

// CHECK: 1

