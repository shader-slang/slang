//TEST_IGNORE_FILE:
#version 450 core

struct Test
{
    vec4 a;
};

layout(binding = 0, set = 1)
uniform gTest_S1
{
    Test gTest;
};

layout(binding = 1, set = 1)
uniform texture2D gTest_t;

layout(binding = 2, set = 1)
uniform sampler gTest_s;

vec4 main_(vec2 uv)
{
    return gTest.a + texture(sampler2D(gTest_t, gTest_s), uv);
}

layout(location = 0)
in vec2 SLANG_in_uv;

layout(location = 0)
out vec4 SLANG_out_main_result;

void main()
{
    vec2 uv = SLANG_in_uv;
    vec4 main_result = main_(uv);
    SLANG_out_main_result = main_result;
}
