#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 3 "tests/bindings/glsl-parameter-blocks.slang"
struct Test_0
{
    vec4 a_0;
};


#line 7
layout(binding = 0)
layout(std140) uniform _S1
{
    vec4 a_0;
}gTest_0;

#line 3
layout(binding = 1)
uniform texture2D gTest_t_0;


#line 1237 "core.meta.slang"
layout(binding = 2)
uniform sampler gTest_s_0;


#line 89 "core"
layout(location = 0)
out vec4 _S2;


#line 902 "core.meta.slang"
layout(location = 0)
in vec2 _S3;


#line 12 "tests/bindings/glsl-parameter-blocks.slang"
void main()
{
    vec4 _S4 = (texture(sampler2D(gTest_t_0,gTest_s_0), (_S3)));

#line 14
    _S2 = gTest_0.a_0 + _S4;

#line 14
    return;
}

