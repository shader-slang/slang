//TEST_IGNORE_FILE:

#version 450

#extension GL_EXT_nonuniform_qualifier : require

struct SLANG_ParameterGroup_C_0
{
    vec2 uv_0;
    uint index_0;
};

layout(binding = 2)
layout(std140)
uniform _S1
{
    SLANG_ParameterGroup_C_0 _data;
} C_0;

layout(binding = 0)
uniform texture2D t_0[];

layout(binding = 1)
uniform sampler s_0;

layout(location = 0)
out vec4 _S2;

void main()
{
    vec4 _S3 = texture(
    	sampler2D(
    		t_0[C_0._data.index_0],
    		s_0),
		C_0._data.uv_0);
    _S2 = _S3;
    return;
}