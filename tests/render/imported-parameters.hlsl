//TEST(smoke,render):COMPARE_HLSL_GLSL_RENDER:

// This test is trying to ensure that we can
// correctly handle cases where top-level shader
// parameters are declared in an `import`ed file.

// Pull in Spire code depdendency using extended syntax:
__import imported_parameters;

#if defined(__HLSL__)

struct AssembledVertex
{
	float3	position;
	float3	color;
};

struct CoarseVertex
{
	float3	color;
};

struct Fragment
{
	float4 color;
};

// Vertex  Shader

struct VertexStageInput
{
	AssembledVertex assembledVertex	: A;
};

struct VertexStageOutput
{
	CoarseVertex	coarseVertex	: CoarseVertex;
	float4			sv_position		: SV_Position;
};

VertexStageOutput vertexMain(VertexStageInput input)
{
	VertexStageOutput output;

	float3 position = input.assembledVertex.position;
	float3 color	= input.assembledVertex.color;

	output.coarseVertex.color = color;
	output.sv_position = mul(modelViewProjection, float4(position, 1.0));

	return output;
	
}

// Fragment Shader

struct FragmentStageInput
{
	CoarseVertex	coarseVertex	: CoarseVertex;
};

struct FragmentStageOutput
{
	Fragment fragment	: SV_Target;
};

FragmentStageOutput fragmentMain(FragmentStageInput input)
{
	FragmentStageOutput output;

	float3 color = input.coarseVertex.color;

	color = transformColor(color);

	output.fragment.color = float4(color, 1.0);

	return output;
}

#elif defined(__GLSL__)

#version 420

#define ASSEMBLED_VERTEX(QUAL)		\
	/* */

#define V2F(QUAL)			\
	QUAL vec3 coarse_color;	\
	/* */

// Vertex  Shader

#ifdef __GLSL_VERTEX__

layout(location = 0)
in vec3 assembled_position;

layout(location = 1)
in vec3 assembled_color;

V2F(out)

void main()
{
	vec3 position = assembled_position;
	vec3 color	= assembled_color;

	coarse_color = color;
//	gl_Position = modelViewProjection * vec4(position, 1.0);
	gl_Position = vec4(position, 1.0) * modelViewProjection;
}

#endif

#ifdef __GLSL_FRAGMENT__

V2F(in)

layout(location = 0)
out vec4 fragment_color;

void main()
{
	vec3 color = coarse_color;

	color = transformColor(color);

	fragment_color = vec4(color, 1.0);
}


#endif

#endif
