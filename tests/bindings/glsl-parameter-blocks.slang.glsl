#version 450
layout(row_major) uniform;
layout(row_major) buffer;
struct Test_0
{
    vec4 a_0;
};

layout(binding = 0)
layout(std140) uniform _S1
{
    Test_0 _data;
} gTest_0;
layout(binding = 1)
uniform texture2D gTest_t_0;

layout(binding = 2)
uniform sampler gTest_s_0;

layout(location = 0)
out vec4 _S2;

layout(location = 0)
in vec2 _S3;

void main()
{
    vec4 _S4 = (texture(sampler2D(gTest_t_0,gTest_s_0), (_S3)));
    _S2 = gTest_0._data.a_0 + _S4;
    return;
}
