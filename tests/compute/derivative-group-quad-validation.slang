//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm
//TEST_INPUT:ubuffer(data=[0], stride=4):out

// Test that DerivativeGroupQuadsKHR execution mode validates workgroup size
// This test should produce an error because numthreads(1,1,1) violates the requirement
// that X and Y dimensions must be multiples of 2

[[vk::binding(0, 0)]]
RWStructuredBuffer<float4> g_ssbo;

[[vk::binding(0, 1)]]
Texture2D g_textures[];
[[vk::binding(0, 2)]]
SamplerState g_samplers[];

[shader("compute")]
[numthreads(1, 1, 1)]  // Invalid: should be multiples of 2 for derivative group quads
void main() {
    float4 texture_color = g_textures[0].Sample(
        g_samplers[0],
        float2(0.0f, 0.0f)
    );
    g_ssbo[0] = texture_color;
}

// CHECK: error 31210: compute derivative group quad requires thread dispatch count of X and Y to each be at a multiple of 2