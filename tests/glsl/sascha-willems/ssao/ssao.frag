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

LAYOUT(binding = 0) uniform sampler2D samplerPositionDepth;
LAYOUT(binding = 1) uniform sampler2D samplerNormal;
LAYOUT(binding = 2) uniform sampler2D ssaoNoise;

layout (constant_id = 0) const int SSAO_KERNEL_SIZE = 64;
layout (constant_id = 1) const float SSAO_RADIUS = 0.5;

LAYOUT(binding = 3) uniform UBOSSAOKernel
{
	vec4 samples[SSAO_KERNEL_SIZE];
} uboSSAOKernel;

LAYOUT(binding = 4) uniform UBO 
{
	mat4 projection;
} ubo;

LAYOUT(location = 0) in vec2 inUV;

LAYOUT(location = 0) out float outFragColor;

void main() 
{
	// Get G-Buffer values
	vec3 fragPos = texture(samplerPositionDepth, inUV).rgb;
	vec3 normal = normalize(texture(samplerNormal, inUV).rgb * 2.0 - 1.0);

	// Get a random vector using a noise lookup
	ivec2 texDim = textureSize(samplerPositionDepth, 0); 
	ivec2 noiseDim = textureSize(ssaoNoise, 0);
	const vec2 noiseUV = vec2(float(texDim.x)/float(noiseDim.x), float(texDim.y)/(noiseDim.y)) * inUV;  
	vec3 randomVec = texture(ssaoNoise, noiseUV).xyz * 2.0 - 1.0;
	
	// Create TBN matrix
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(tangent, normal);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// Calculate occlusion value
	float occlusion = 0.0f;
	for(int i = 0; i < SSAO_KERNEL_SIZE; i++)
	{		
		vec3 samplePos = TBN * uboSSAOKernel.samples[i].xyz; 
		samplePos = fragPos + samplePos * SSAO_RADIUS; 
		
		// project
		vec4 offset = vec4(samplePos, 1.0f);
		offset = ubo.projection * offset; 
		offset.xyz /= offset.w; 
		offset.xyz = offset.xyz * 0.5f + 0.5f; 
		
		float sampleDepth = -texture(samplerPositionDepth, offset.xy).w; 

#define RANGE_CHECK 1
#ifdef RANGE_CHECK
		// Range check
		float rangeCheck = smoothstep(0.0f, 1.0f, SSAO_RADIUS / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z ? 1.0f : 0.0f) * rangeCheck;           
#else
		occlusion += (sampleDepth >= samplePos.z ? 1.0f : 0.0f);  
#endif
	}
	occlusion = 1.0 - (occlusion / float(SSAO_KERNEL_SIZE));
	
	outFragColor = occlusion;
}

