// vk-push-constant.slang.glsl
#version 450

struct S_0
{
    vec4 v_0;
};

layout(push_constant)
layout(std140) uniform _S1
{
    S_0 _data;
} x_0;

layout(binding = 0, set = 0)
layout(std140) uniform _S2
{
    S_0 _data;
} y_0;

layout(location = 0)
out vec4 _S3;

void main()
{
    _S3 = x_0._data.v_0 + y_0._data.v_0;
    return;
}
