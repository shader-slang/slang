//TEST_IGNORE_FILE:
#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 5 0
layout(location = 0)
out vec4 _S1;

#line 5
layout(early_fragment_tests) in;
void main()
{
    _S1 = vec4(float(1), float(0), float(0), float(1));
    return;
}
