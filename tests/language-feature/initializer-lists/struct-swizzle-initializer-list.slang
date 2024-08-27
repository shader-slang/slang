//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

//CHECK: computeMain

RWStructuredBuffer<uint> outputBuffer;

struct Test
{
    float3 a;
    float3 b;
    float2 c;
};

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float3 position = {1, 2, 3};
    const Test test[3] = 
    {
        mul(float4x4(1), float4(position, 1.f)).xyz, mul(float4x4(1), float4(position, 1.f)).xyz - mul(float4x4(1), float4(position, 1.f)).xyz, float2(2),
        mul(float4x4(1), float4(position, 1.f)).xyz, mul(float4x4(1), float4(position, 1.f)).xyz - mul(float4x4(1), float4(position, 1.f)).xyz, float2(2),
        mul(float4x4(1), float4(position, 1.f)).xyz, mul(float4x4(1), float4(position, 1.f)).xyz - mul(float4x4(1), float4(position, 1.f)).xyz, float2(2)
    };
    outputBuffer[0] = true
        && test[0].a[0] != 99
        && test[0].b[0] != 99
        && test[1].c[0] != 99
        ;
}