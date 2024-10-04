//TEST:SIMPLE(filecheck=CHECK): -target dxil -entry computeMain -stage compute -profile sm_6_5
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -shaderobj

struct TestInner
{
    float4 a = {};
};

TestInner makeTestInner(float v)
{
    TestInner val;
    val.a = (float4)v;
    return val;
}

struct Test
{
    float3 a;
    float3 b;
    TestInner c;
};


//CHECK: computeMain

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test test = { float3(1, 2, 3), (float3)2, makeTestInner(3) };

//BUF: 1
    outputBuffer[0] = true
        && all(test.a == float3(1, 2, 3))
        && all(test.b == float3(2, 2, 2))
        && all(test.c.a == float4(3, 3, 3, 3))
        ;
}
