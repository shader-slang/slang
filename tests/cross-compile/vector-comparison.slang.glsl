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
    _S2 = mix(vec4(3.0), vec4(2.0), (equal(params_0._data.a_0,params_0._data.b_0))) + mix(vec4(3.0), vec4(2.0), (lessThan(params_0._data.a_0,params_0._data.b_0))) + mix(vec4(3.0), vec4(2.0), (greaterThan(params_0._data.a_0,params_0._data.b_0))) + mix(vec4(3.0), vec4(2.0), (lessThanEqual(params_0._data.a_0,params_0._data.b_0))) + mix(vec4(3.0), vec4(2.0), (greaterThanEqual(params_0._data.a_0,params_0._data.b_0))) + mix(vec4(3.0), vec4(2.0), (notEqual(params_0._data.a_0,params_0._data.b_0)));
    return;
}
