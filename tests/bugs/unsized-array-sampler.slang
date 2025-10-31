//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -profile cs_6_0

// CHECK: Texture2D{{.*}} textures_texture{{.*}}[] : register(t0, space2);
// CHECK: SamplerState{{.*}} textures_sampler{{.*}}[] : register(s0, space2);

Sampler2D textures[] : register(t0, space2) : register(s0, space2);
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    float4 sum = 0;
    sum += textures[0].SampleLevel(float2(0, 0), 0);
    outputBuffer[0] = sum;
}