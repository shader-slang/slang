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
    vec4 a_0;
    vec4 b_0;
} params_0;
layout(location = 0)
out vec4 main_0;

void main()
{

    const vec4 _S3 = vec4(2.0);
    const vec4 _S4 = vec4(3.0);
    main_0 = mix(_S4, _S3, (equal(params_0.a_0,params_0.b_0))) + mix(_S4, _S3, (lessThan(params_0.a_0,params_0.b_0))) + mix(_S4, _S3, (greaterThan(params_0.a_0,params_0.b_0))) + mix(_S4, _S3, (lessThanEqual(params_0.a_0,params_0.b_0))) + mix(_S4, _S3, (greaterThanEqual(params_0.a_0,params_0.b_0))) + mix(_S4, _S3, (notEqual(params_0.a_0,params_0.b_0)));
    return;
}
