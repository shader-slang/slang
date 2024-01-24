// sv-coverage.slang.glsl
#version 450

layout(location = 0)
out vec4 main_0;

layout(location = 0)
in vec4 color_0;

void main()
{
    uint _S1 = uint(gl_SampleMaskIn[0]) ^ uint(1);
    main_0 = color_0;
    gl_SampleMask[0] = int(_S1);
    return;
}
