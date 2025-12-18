//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm
//TEST_INPUT:ubuffer(data=[0], stride=4):out

// Test that DerivativeGroupLinearKHR execution mode validates workgroup size
// This test should produce an error because numthreads(3,1,1) violates the requirement
// that the product of dimensions must be a multiple of 4

[[vk::binding(0, 0)]]
RWStructuredBuffer<float4> g_ssbo;

[[vk::binding(0, 1)]]
Texture2D g_textures[];
[[vk::binding(0, 2)]]
SamplerState g_samplers[];

[shader("compute")]
[DerivativeGroupLinear]
[numthreads(3, 1, 1)]  // Invalid: 3*1*1 = 3, not multiple of 4
void main() {
    float4 texture_color = g_textures[0].Sample(
        g_samplers[0],
        float2(0.0f, 0.0f)
    );
    g_ssbo[0] = texture_color;
}

// CHECK: error 31211: compute derivative group linear requires total thread dispatch count to be at a multiple of 4