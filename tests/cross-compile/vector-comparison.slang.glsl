//TEST_IGNORE_FILE
#version 450
layout(row_major) uniform;
layout(row_major) buffer;

struct Param_0
{
    vec4 a_0;
    vec4 b_0;
};

layout(binding = 0)
layout(std140) uniform _S1
{
    Param_0 _data;
} params_0;
layout(location = 0)
out vec4 _S2;

void main()
{
    vec4 v0_0 = params_0._data.a_0;
    vec4 v1_0 = params_0._data.b_0;
    _S2 = mix(vec4(3.00000000000000000000), vec4(2.00000000000000000000), equal(v0_0,v1_0)) + mix(vec4(3.00000000000000000000), vec4(2.00000000000000000000), lessThan(v0_0,v1_0)) + mix(vec4(3.00000000000000000000), vec4(2.00000000000000000000), greaterThan(v0_0,v1_0)) + mix(vec4(3.00000000000000000000), vec4(2.00000000000000000000), lessThanEqual(v0_0,v1_0)) + mix(vec4(3.00000000000000000000), vec4(2.00000000000000000000), greaterThanEqual(v0_0,v1_0)) + mix(vec4(3.00000000000000000000), vec4(2.00000000000000000000), notEqual(v0_0,v1_0));
    return;
}
