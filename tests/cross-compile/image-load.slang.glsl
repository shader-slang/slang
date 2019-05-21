// image-load.slang.glsl

#version 450

#extension GL_EXT_samplerless_texture_functions : require

layout(r32f)
layout(binding = 0)
uniform image2DArray gParams_tex_0;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main()
{
    uvec3 _S1 = uvec3(gl_GlobalInvocationID);

    float _S2 = imageLoad(
    	gParams_tex_0,
    	ivec3(ivec2(_S1.xy), int(_S1.z))).x;

    return;
}
