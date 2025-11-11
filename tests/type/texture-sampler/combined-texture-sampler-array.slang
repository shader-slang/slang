//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry fragMain -profile ps_6_0

// Test for issue #7246
// This test verifies that register attributes are properly preserved when lowering 
// combined texture+sampler arrays into separate texture and sampler arrays for all sampler types.

// Sampler2D arrays with both texture and sampler register attributes
// CHECK-DAG: Texture2D<float4 > tex2D_texture_0[int(16)] : register(t0, space2);
// CHECK-DAG: SamplerState tex2D_sampler_0[int(16)] : register(s0, space2);
Sampler2D tex2D[16] : register(t0, space2) : register(s0, space2);

// SamplerCube arrays
// CHECK-DAG: TextureCube<float4 > texCube_texture_0[int(8)] : register(t5, space1);
// CHECK-DAG: SamplerState texCube_sampler_0[int(8)] : register(s10, space1);
SamplerCube texCube[8] : register(t5, space1) : register(s10, space1);

// Sampler3D arrays with default space (space0)
// CHECK-DAG: Texture3D<float4 > tex3D_texture_0[int(4)] : register(t20);
// CHECK-DAG: SamplerState tex3D_sampler_0[int(4)] : register(s25);
Sampler3D tex3D[4] : register(t20) : register(s25);

// Sampler1D arrays
// CHECK-DAG: Texture1D<float4 > tex1D_texture_0[int(2)] : register(t30, space3);
// CHECK-DAG: SamplerState tex1D_sampler_0[int(2)] : register(s30, space3);
Sampler1D tex1D[2] : register(t30, space3) : register(s30, space3);

// Single element arrays
// CHECK-DAG: Texture2D<float4 > singleTex_texture_0[int(1)] : register(t100, space5);
// CHECK-DAG: SamplerState singleTex_sampler_0[int(1)] : register(s100, space5);
Sampler2D singleTex[1] : register(t100, space5) : register(s100, space5);

// Large array to test with bigger sizes
// CHECK-DAG: Texture2D<float4 > largeTex_texture_0[int(64)] : register(t200, space10);
// CHECK-DAG: SamplerState largeTex_sampler_0[int(64)] : register(s200, space10);
Sampler2D largeTex[64] : register(t200, space10) : register(s200, space10);

// Mix with non-array combined sampler to ensure regression doesn't affect single samplers
// CHECK-DAG: Texture2D<float4 > singleSampler_texture{{.*}} : register(t300);
// CHECK-DAG: SamplerState singleSampler_sampler{{.*}} : register(s300);
Sampler2D singleSampler : register(t300) : register(s300);

// Array with only texture register (to test partial register specification)
// CHECK-DAG: Texture2D<float4 > partialTex_texture_0[int(2)] : register(t400);
// CHECK-DAG: SamplerState partialTex_sampler_0[int(2)] : register(s0);
Sampler2D partialTex[2] : register(t400);

float4 fragMain() : SV_Target
{
    // Sample from various arrays to ensure they're used
    float4 result = tex2D[0].Sample(float2(0.5, 0.5));
    result += texCube[1].Sample(float3(0.2, 0.3, 0.4));
    result += tex3D[2].Sample(float3(0.1, 0.9, 0.6));
    result += tex1D[1].Sample(0.7);
    result += singleTex[0].Sample(float2(0.7, 0.4));
    result += largeTex[5].Sample(float2(0.8, 0.6));
    result += singleSampler.Sample(float2(0.3, 0.7));
    result += partialTex[1].Sample(float2(0.4, 0.8));
    
    return result;
} 