//TEST_IGNORE_FILE:
#version 450
layout(row_major) uniform;
layout(row_major) buffer;

struct P_0
{
    vec4 param_0;
};

layout(binding = 0)
layout(std140) uniform _S1
{
    P_0 _data;
} pblock_0;
layout(location = 0)
out vec4 _S2;

void main()
{
    _S2 = pblock_0._data.param_0;
    return;
}