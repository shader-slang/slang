// structured-buffer-load.slang

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-mtl -compute -shaderobj

// readBuffer.Load(0)=3, readBuffer[1]=5, readWriteBuffer.Load(2)=7, readWriteBuffer[3]=49 -> sum=64=0x40
//TEST_INPUT:ubuffer(data=[3 5 0 0], stride=4):name=readBuffer
StructuredBuffer<uint> readBuffer;

//TEST_INPUT:ubuffer(data=[0 0 7 49], stride=4):name=readWriteBuffer
RWStructuredBuffer<uint> readWriteBuffer;

// int4 variant to exercise non-scalar element type on WGSL/Metal:
// element[0]=(30,0,0,0), element[1]=(0,25,0,0) -> a.x+b.y=55=0x37
//TEST_INPUT:ubuffer(data=[30 0 0 0  0 25 0 0], stride=16):name=int4ReadBuffer
StructuredBuffer<int4> int4ReadBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
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
    //CHK:40

    outputBuffer[1] = outputBuffer.Load(0);
    //CHK-NEXT:40

    int4 a = int4ReadBuffer.Load(0);
    int4 b = int4ReadBuffer[1];
    outputBuffer[2] = uint(a.x + b.y);
    //CHK-NEXT:37
}
