// texture2d-ms.hlsl.glsl
//TEST_IGNORE_FILE:

#version 450
layout(row_major) uniform;
layout(row_major) buffer;
#extension GL_EXT_samplerless_texture_functions : require
layout(binding = 0)
uniform texture2DMS tex_0;

layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;void main()
{
    vec4 _S1 = (texelFetch((tex_0), (ivec2(gl_WorkGroupID.xy)), (0)));
    return;
}

