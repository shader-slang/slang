// structured-buffer-load.slang

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-mtl -compute -shaderobj

//TEST_INPUT:ubuffer(data=[1 2 9 9], stride=4):name=readBuffer
StructuredBuffer<uint> readBuffer;

//TEST_INPUT:ubuffer(data=[9 9 3 4], stride=4):name=readWriteBuffer
RWStructuredBuffer<uint> readWriteBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint result = readBuffer.Load(0);
    result += readBuffer[1];
    result += readWriteBuffer.Load(2);
    result += readWriteBuffer[3];

    outputBuffer[0] = result;
    //CHK:A
    outputBuffer[1] = outputBuffer.Load(0);
    //CHK-NEXT:A
}
