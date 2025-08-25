//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -Wno-41024
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -slang -output-using-type -shaderobj -vk -xslang -Wno-41024

//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content = zero, arrayLength=2, mipMaps = 1):name rwTexture2DArray
[[vk::binding(2)]]
RWTexture2DArray<uint> rwTexture2DArray;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint3 loc = (uint2(0), 0);
    uint prevVal;
//CHECK: %{{.*}} = OpImageTexelPointer %{{.*}} %{{.*}} %{{.*}} %int_0
    InterlockedExchange(rwTexture2DArray[loc], 3, prevVal);
    outputBuffer[0] = uint(true
            && rwTexture2DArray[loc] == 3
            && prevVal == 0
            );
}

//BUF: 1
