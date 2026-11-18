//TEST:COMPARE_COMPUTE(targets=vk+dx12+cuda+cpu+llvm, filecheck-buffer=BUF):-entry computeMain

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct TextureHandle
{
    uint32_t packedData = 0;
    __init(uint32_t packedData) { this.packedData = packedData; }
};

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    TextureHandle texHandle = TextureHandle(1);
    // BUF: 1
    outputBuffer[0] = true
        && texHandle.packedData == 1
        ;
}
