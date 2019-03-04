//TEST(smoke):COMPARE_HLSL_RENDER:
//TEST_INPUT: Texture2D(size=16, content=chessboard):dxbinding(0),glbinding(0)

Texture2D<float4> g_texture : register(t0);
SamplerState g_sampler : register(s0);

cbuffer Uniforms
{
	float4x4 modelViewProjection;
}

struct AssembledVertex
{
	float3	position;
	float3	color;
};

// Vertex  Shader
struct VertexStageInput
{
	AssembledVertex assembledVertex	: A;
};

struct VertexStageOutput
{
	float4          color           : COLOR; 
	float4			position		: SV_Position;
};

VertexStageOutput vertexMain(VertexStageInput input) 
{
    VertexStageOutput output;
    
    output.position = mul(modelViewProjection, float4(input.assembledVertex.position, 1.0));
    output.color = float4(input.assembledVertex.color, 1.0f);
    
    return output;
}

// Fragment Shader
float4 fragmentMain(VertexStageOutput input) : SV_Target
{
    //return input.color;

    //float2 samplePos = ((pos.xy * 0.5f) + float2(0.5f, 0.5f));
    //return float4(samplePos, 0, 1.0f);
    //return g_texture.Sample(g_sampler, input.color.xy);
    
    return g_texture.GatherRed(g_sampler, input.color.xy);
}