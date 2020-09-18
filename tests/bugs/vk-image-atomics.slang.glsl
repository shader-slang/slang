#version 450

layout(r32ui)
layout(binding = 0)
uniform uimage2D t_0;

layout(location = 0)
out vec4 _S1;

void main()
{
    uint _S2;
    _S2 = imageAtomicAdd(t_0, ivec2(uvec2(0)), 1);
    _S1 = vec4(_S2);
    return;
}
