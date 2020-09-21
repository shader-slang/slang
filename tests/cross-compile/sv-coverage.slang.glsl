// sv-coverage.slang.glsl
#version 450

layout(location = 0)
out vec4 _S1;

layout(location = 0)
in vec4 _S2;

void main()
{
    uint _S3 = uint(gl_SampleMaskIn[0]) ^ uint(1);
    _S1 = _S2;
    gl_SampleMask[0] = int(_S3);
    return;
}
