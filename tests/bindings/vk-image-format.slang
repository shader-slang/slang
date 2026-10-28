// vk-image-format.slang

//TEST:SIMPLE(filecheck=CHECK):-std 2026 -warnings-as-errors all -target glsl -profile ps_4_0 -entry main -line-directive-mode none
//TEST:SIMPLE(filecheck=SPIRV):-std 2026 -warnings-as-errors all -target spirv-asm -stage fragment -entry main

//CHECK: layout(r32f)
//CHECK: layout(r16_snorm)
//CHECK: layout(r11f_g11f_b10f)
//CHECK: layout(rgba16_snorm)
//CHECK: layout(rgba16f)

// SPIRV: OpTypeImage {{.*}} Rgba16f

// Something typical
[vk::image_format("r32f")]
RWTexture2D<float> typicalTexture;

// snorm
[vk::image_format("r16snorm")]
RWTexture2D<float> snormTexture;

// Special case
[vk::image_format("r11g11b10f")]
RWTexture2D<float4> specialTexture;

// snorm with underscore in format name
[vk::image_format("rgba16_snorm")]
RWTexture2D<float4> snormTextureWithUnderscore;

// Existing attributes override the legacy integer format argument. Format 1 is
// rgba32f; the explicit attribute must still select rgba16f for this texture.
[format("rgba16f")]
RWTexture2D<float4, 0, 1> attributedLegacyTexture;

cbuffer C
{
	uint2 index;
}

float4 main(): SV_Target
{
    float4 result = 0;

    result += typicalTexture[index];
    result += snormTexture[index];
    result += specialTexture[index];
    result += snormTextureWithUnderscore[index];
    result += attributedLegacyTexture[index];
    
    return result;
}
