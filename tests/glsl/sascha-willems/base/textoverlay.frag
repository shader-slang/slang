#version 450 core
//TEST:COMPARE_GLSL:
//TEST:COMPARE_GLSL:-DBINDING

#if defined(__SLANG__) && defined(BINDING)
#define LAYOUT(X) /* empty */
#else
#define LAYOUT(X) layout(X)
#endif

LAYOUT(location = 0) in vec2 inUV;

LAYOUT(binding = 0) uniform sampler2D samplerFont;

LAYOUT(location = 0) out vec4 outFragColor;

void main(void)
{
	float color = texture(samplerFont, inUV).r;
	outFragColor = vec4(vec3(color), 1.0);
}
