#version 450
//TEST:COMPARE_GLSL:
//TEST:COMPARE_GLSL:-DBINDING

#if defined(__SLANG__) && defined(BINDING)
#define LAYOUT(X) /* empty */
#else
#define LAYOUT(X) layout(X)
#endif

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

LAYOUT(location = 0) in vec4 inPos;
LAYOUT(location = 1) in vec2 inUV;
LAYOUT(location = 2) in vec3 inColor;
LAYOUT(location = 3) in vec3 inNormal;

LAYOUT(binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	mat4 model;
} ubo;

LAYOUT(location = 0) out vec3 outNormal;
LAYOUT(location = 1) out vec2 outUV;
LAYOUT(location = 2) out vec3 outColor;
LAYOUT(location = 3) out vec3 outViewVec;
LAYOUT(location = 4) out vec3 outLightVec;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	outNormal = inNormal;
	outColor = inColor;
	outUV = inUV;
	gl_Position = ubo.projection * ubo.view * ubo.model * inPos;

	vec3 lightPos = vec3(-5.0, -5.0, 0.0);
	vec4 pos = ubo.view * ubo.model * inPos;
	outNormal = mat3(ubo.view * ubo.model) * inNormal;
	outLightVec = lightPos - pos.xyz;
	outViewVec = -pos.xyz;	
}
