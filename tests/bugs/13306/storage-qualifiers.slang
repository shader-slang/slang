//TEST:COMPILE: -lang hlsl tests/bugs/13306/storage-qualifiers.slang -Gec -target hlsl -entry main -validate-ir
//TEST:COMPILE: -lang hlsl tests/bugs/13306/storage-qualifiers.slang -Gec -target spirv -entry main -validate-ir

// Existing global storage and constants keep their behavior. Specialization constants and
// unbounded resource arrays stay inputs even when the compatibility option is enabled.
static uint counter = 1;
static const uint size = 2;
groupshared uint sharedValue;
[[vk::constant_id(0)]] const uint specialization = 3;
Texture2D<float4> textures[];
RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    counter++;
    sharedValue = tid.x;
    output[tid.x] = textures[0].Load(int3(0)) + float4(counter + size + specialization + sharedValue);
}
