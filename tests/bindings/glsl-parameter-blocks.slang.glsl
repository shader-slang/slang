//TEST_IGNORE_FILE:
#version 450 core

#define Test Test_0
#define a a_0

#define gTest gTest_0
#define gTest_t gTest_t_0
#define gTest_s gTest_s_0

#define ParameterBlock_gTest _S1

#define main_result _S2
#define uv _S3

#define temp_uv _S4
#define temp_a _S5
#define temp_sample _S6
#define temp_add _S7

struct Test
{
    vec4 a;
};

layout(binding = 0, set = 1)
uniform ParameterBlock_gTest
{
    Test gTest;
};

layout(binding = 1, set = 1)
uniform texture2D gTest_t;

layout(binding = 2, set = 1)
uniform sampler gTest_s;

layout(location = 0)
out vec4 main_result;

layout(location = 0)
in vec2 uv;

void main()
{
	vec2 temp_uv = uv;

	vec4 temp_a = gTest.a;

    vec4 temp_sample = texture(sampler2D(gTest_t, gTest_s), temp_uv);

    vec4 temp_add = temp_a + temp_sample;
	main_result = temp_add;

	return;
}
