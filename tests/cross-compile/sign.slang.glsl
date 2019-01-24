//TEST_IGNORE_FILE:
#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 8 0
layout(location = 0)
out vec4 _S1;


#line 8
void main()
{
    ivec4 _S2 = ivec4(sign(vec4(1.50000000000000000000, 1.00000000000000000000, -1.50000000000000000000, -1.00000000000000000000)));
    _S1 = vec4(_S2);
    return;
}