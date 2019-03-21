//TEST(compute):COMPARE_RENDER_COMPUTE:

//TEST_INPUT: Texture1D(size=4, content = one) : dxbinding(0),glbinding(0)
//TEST_INPUT: Texture2D(size=4, content = one) : dxbinding(1),glbinding(1)
//TEST_INPUT: Texture3D(size=4, content = one) : dxbinding(2),glbinding(2)
//TEST_INPUT: TextureCube(size=4, content = one) : dxbinding(3),glbinding(3)
//TEST_INPUT: Texture1D(size=4, content = one, arrayLength=2) : dxbinding(4),glbinding(4)
//TEST_INPUT: Texture2D(size=4, content = one, arrayLength=2) : dxbinding(5),glbinding(5)
//TEST_INPUT: TextureCube(size=4, content = one, arrayLength=2) : dxbinding(6),glbinding(6)
//TEST_INPUT: Sampler : dxbinding(0),glbinding(0,1,2,3,4,5,6)
//TEST_INPUT: ubuffer(data=[0], stride=4):dxbinding(1),glbinding(0),out

Texture1D t1D;
Texture2D t2D;
Texture3D t3D;
TextureCube tCube;
Texture1DArray t1dArray;
Texture2DArray t2dArray;
TextureCubeArray tCubeArray;
SamplerState samplerState;
RWStructuredBuffer<float> outputBuffer;

cbuffer Uniforms
{
	float4x4 modelViewProjection;
}

struct AssembledVertex
{
	float3	position;
	float3	color;
    float2  uv;
};

struct CoarseVertex
{
	float3	color;
    float2  uv;
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
    output.coarseVertex.uv = input.assembledVertex.uv;
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
    float2 uv = input.coarseVertex.uv;
	output.fragment.color = float4(color, 1.0);

    float4 val = 0.0;
    val += t1D.Sample(samplerState, uv.x);
    val += t2D.Sample(samplerState, uv);
    val += t3D.Sample (samplerState, float3(uv, 0.5));
    val += t1dArray.Sample(samplerState, float2(uv.x, 0.0));
    
    val += t2dArray.Sample(samplerState, float3(uv, 0.0));
    
    val += tCubeArray.Sample(samplerState, float4(uv, 0.5, 0.0));
    val += tCube.Sample(samplerState, float3(uv, 0.5));
    
    outputBuffer[0] = val.x;
	return output;
}
