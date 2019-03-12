#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout(binding = 0)
uniform texture2D  gParams_textures_0[10];

float fetchData_0(uvec2 coords_0, uint index_0)
{
    float _S1 = texelFetch(
    	gParams_textures_0[nonuniformEXT(index_0)],
    	ivec2(coords_0),
    	0).x;

    return _S1;
}

layout(location = 0)
out vec4 _S2;

flat layout(location = 0)
in uvec3 _S3;

void main()
{
    float v_0 = fetchData_0(_S3.xy, _S3.z);
    _S2 = vec4(v_0);
    return;
}
