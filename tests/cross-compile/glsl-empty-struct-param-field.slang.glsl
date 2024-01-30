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
    vec4 param_0;
} pblock_0;
layout(location = 0)
out vec4 main_0;

void main()
{
    main_0 = pblock_0.param_0;
    return;
}