//TEST_IGNORE_FILE:
#version 450 core

struct _ST04Test
{
    vec4 a;
};

layout(binding = 0, set = 1)
uniform _S1
{
    _ST04Test _SV05gTestL0;
};

layout(binding = 1, set = 1)
uniform texture2D _SV05gTestL1;

layout(binding = 2, set = 1)
uniform sampler _SV05gTestL2;

layout(location = 0)
in vec2 _S3;

layout(location = 0)
out vec4 _S2;

void main()
{
	vec2 _S4 = _S3;

	vec2 _S5 = _S4;

	vec4 _S6 = _SV05gTestL0.a;

	vec2 _S7 = _S5;


    vec4 _S8 = texture(sampler2D(_SV05gTestL1, _SV05gTestL2), _S7);

    vec4 _S9 = _S6 + _S8;
	_S2 = _S9;

	return;
}
