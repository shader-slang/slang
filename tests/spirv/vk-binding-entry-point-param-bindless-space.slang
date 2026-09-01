//TEST:REFLECTION(filecheck=REFLECTION): -target spirv-asm -entry main -stage compute

// REFLECTION: "bindlessSpaceIndex": 0

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(3, 2)]]
    StructuredBuffer<float> input,
    [[vk::binding(4, 2)]]
    RWStructuredBuffer<float> output,
    uint3 tid : SV_DispatchThreadID)
{
    Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(uint2(tid.x, 0));
    output[tid.x] = input[tid.x] + texture.Load(int3(0, 0, 0)).x;
}
