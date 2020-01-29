// texture-load.slang.glsl
//TEST_IGNORE_FILE:

#version 450

layout(row_major) uniform;
layout(row_major) buffer;

#extension GL_EXT_samplerless_texture_functions : require

struct SLANG_ParameterGroup_C_0
{
    ivec2 pos_0;
};

layout(binding = 2)
layout(std140)
uniform _S1
{
    SLANG_ParameterGroup_C_0 _data;
} C_0;

layout(binding = 0)
uniform texture2D inputTexture_0;

layout(rg32f)
layout(binding = 1)
uniform image2D outputTexture_0;

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
    ivec3 _S2 = ivec3(C_0._data.pos_0, 0);

    vec2 tmp_0 = texelFetch(
    	inputTexture_0,
    	_S2.xy,
    	_S2.z).xy;

    imageStore(
    	outputTexture_0,
    	ivec2(uvec2(C_0._data.pos_0)),
    	vec4(tmp_0, float(0), float(0)));

    return;
}
