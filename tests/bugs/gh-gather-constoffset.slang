//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry main -stage fragment

// Test that Gather with a constant offset uses `ConstOffset` instead of `Offset`
// and does NOT require ImageGatherExtended capability.

//SPV-NOT:OpCapability ImageGatherExtended

[[vk::binding(0, 0)]]
Texture2D<float> tex;

[[vk::binding(1, 0)]]
SamplerState s;

[shader("fragment")]
float4 main([[vk::location(0)]] float2 uv : UV) : SV_Target
{
    //SPV:OpImageGather %
    //SPV-SAME: ConstOffset %
    return tex.Gather(s, uv, int2(2, 1));
}
