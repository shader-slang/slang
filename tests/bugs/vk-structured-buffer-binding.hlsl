//TEST:SIMPLE: -profile ps_4_0 -entry main -target glsl
///////TEST:REFLECTION:-profile ps_4_0 -target spirv

[[vk::binding(3, 4)]]
RWStructuredBuffer<uint> gDoneGroups : register(u3);

float4 main(
    float3 uv : UV)
    : SV_Target
{
    return gDoneGroups[int(uv.z)]; 
}