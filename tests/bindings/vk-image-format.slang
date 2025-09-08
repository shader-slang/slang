// vk-image-format.slang

//TEST:SIMPLE(filecheck=CHECK):-target glsl -profile ps_4_0 -entry main -line-directive-mode none

//CHECK: layout(r32f)
//CHECK: layout(r16_snorm)
//CHECK: layout(r11f_g11f_b10f)
//CHECK: layout(rgba16_snorm)

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
    
    return result;
}
