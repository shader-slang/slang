#version 450
#extension GL_EXT_samplerless_texture_functions : require
#extension GL_EXT_nonuniform_qualifier : require
layout(row_major) uniform;
layout(row_major) buffer;

layout(binding = 0)
uniform texture2D  gParams_textures_0[10];


float fetchData_0(uvec2 coords_0, uint index_0)
{
    float _S1 = (texelFetch((gParams_textures_0[nonuniformEXT(index_0)]), ivec2((coords_0)), 0).x);

    return _S1;
}

layout(location = 0)
out vec4 _S2;


flat layout(location = 0)
in uvec3 _S3;


void main()
{

    _S2 = vec4(fetchData_0(_S3.xy, _S3.z));

    return;
}

