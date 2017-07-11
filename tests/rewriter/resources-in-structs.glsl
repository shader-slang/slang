#version 450 core
//TEST:COMPARE_GLSL:-profile glsl_fragment

#if defined(__SLANG__)

__import resources_in_structs;

uniform U
{
	Material m;
};

in vec2 uv;

out vec4 color;

void main()
{
	color = evaluateMaterial(m, uv);
}

#else

struct Material
{
	vec4 color;
};

vec4 evaluateMaterial(
	Material 	m,
	texture2D 	m_t,
	sampler		m_s,
	vec2 		uv)
{
	return m.color + texture(sampler2D(m_t, m_s), uv);
}

layout(binding = 0)
uniform U
{
	Material m;
};

layout(binding = 1)
uniform texture2D SLANG_parameterBlock_U_m_t;

layout(binding = 2)
uniform sampler SLANG_parameterBlock_U_m_s;

in vec2 uv;

out vec4 color;

void main()
{
	color = evaluateMaterial(
		m,
		SLANG_parameterBlock_U_m_t,
		SLANG_parameterBlock_U_m_s, uv);
}

#endif
