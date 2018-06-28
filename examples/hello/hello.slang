// hello.slang

// This file provides a simple vertex and fragment shader that can be compiled
// using Slang. This code should also be valid as HLSL, and thus it does not
// use any of the new language features supported by Slang.

cbuffer Uniforms
{
    float4x4 modelViewProjection;
}

struct AssembledVertex
{
    float3	position : POSITION;
    float3	color    : COLOR;
};

struct CoarseVertex
{
    float3 color;
};

struct Fragment
{
    float4 color;
};


// Vertex  Shader

struct VertexStageInput
{
    AssembledVertex assembledVertex;
};

struct VertexStageOutput
{
    CoarseVertex    coarseVertex    : CoarseVertex;
    float4          sv_position     : SV_Position;
};

VertexStageOutput vertexMain(VertexStageInput input)
{
    VertexStageOutput output;

    float3 position = input.assembledVertex.position;
    float3 color    = input.assembledVertex.color;

    output.coarseVertex.color = color;
    output.sv_position = mul(modelViewProjection, float4(position, 1.0));

    return output;
}

// Fragment Shader

struct FragmentStageInput
{
    CoarseVertex    coarseVertex    : CoarseVertex;
};

struct FragmentStageOutput
{
    Fragment fragment   : SV_Target;
};

FragmentStageOutput fragmentMain(FragmentStageInput input)
{
    FragmentStageOutput output;

    float3 color = input.coarseVertex.color;

    output.fragment.color = float4(color, 1.0);

    return output;
}
