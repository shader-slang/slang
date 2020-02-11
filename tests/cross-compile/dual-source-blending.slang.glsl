//dual-source-blending.slang.glsl
#version 450

layout(row_major) uniform;
layout(row_major) buffer;

layout(location = 0)
out vec4 _S1;

layout(location = 0, index = 1)
out vec4 _S2;

layout(location = 0)
in vec4 _S3;

struct FragmentOutput_0
{
    vec4 a_0;
    vec4 b_0;
};

void main()
{
    FragmentOutput_0 f_0;

    FragmentOutput_0 _S4 = { vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0) };
    f_0 = _S4;

    f_0.a_0 = _S3;
    f_0.b_0 = _S3;

    FragmentOutput_0 _S5 = f_0;
    _S1 = _S5.a_0;
    _S2 = _S5.b_0;

    return;
}
