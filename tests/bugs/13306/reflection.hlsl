//TEST:REFLECTION: -Gec -target hlsl -entry main -no-codegen
//TEST:REFLECTION: -target hlsl -entry main -no-codegen

// Both modes must describe the same shader inputs, with the original names and bindings.
uniform uint x;
cbuffer Settings : register(b2)
{
    uint y;
    float4 z;
};
Texture2D<float4> gTex : register(t3);
RWStructuredBuffer<float4> output : register(u1);

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = gTex.Load(int3(0)) + float4(x + y) + z;
}
