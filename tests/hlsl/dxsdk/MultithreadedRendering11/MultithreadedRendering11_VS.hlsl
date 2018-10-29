//TEST:COMPARE_HLSL: -profile vs_4_0 -entry VSMain

#ifndef __SLANG__
#define cbPerObject cbPerObject_0
#define g_mWorld g_mWorld_0
#define cbPerScene cbPerScene_0
#define g_mViewProj g_mViewProj_0
#endif

//--------------------------------------------------------------------------------------
// File: MultithreadedRendering11_VS.hlsl
//
// The vertex shader file for the MultithreadedRendering11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// Various debug options
//#define UNCOMPRESSED_VERTEX_DATA  // The sdkmesh file contained uncompressed vertex data

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	matrix		g_mWorld	;//SLANG: : packoffset( c0 );
};
cbuffer cbPerScene : register( b1 )
{
	matrix		g_mViewProj	;//SLANG: : packoffset( c0 );
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
	float3 vTangent		: TANGENT;
};

struct VS_OUTPUT
{
	float3 vNormal		: NORMAL;
	float3 vTangent		: TANGENT;
	float2 vTexcoord	: TEXCOORD0;
	float4 vPosWorld	: TEXCOORD1;
	float4 vPosition	: SV_POSITION;
};

// We aliased signed vectors as a unsigned format. 
// Need to recover signed values.  The values 1.0 and 2.0
// are slightly inaccurate here.
float3 R10G10B10A2_UNORM_TO_R32G32B32_FLOAT( in float3 vVec )
{
    vVec *= 2.0f;
    return vVec >= 1.0f ? ( vVec - 2.0f ) : vVec;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
#ifndef UNCOMPRESSED_VERTEX_DATA
	// Expand compressed vectors
	Input.vNormal = R10G10B10A2_UNORM_TO_R32G32B32_FLOAT( Input.vNormal );
	Input.vTangent = R10G10B10A2_UNORM_TO_R32G32B32_FLOAT( Input.vTangent );
#endif  // #ifndef UNCOMPRESSED_VERTEX_DATA
	
	Output.vPosWorld = mul( Input.vPosition,    g_mWorld );
	Output.vPosition = mul( Output.vPosWorld,   g_mViewProj );
	Output.vNormal   = mul( Input.vNormal,      (float3x3)g_mWorld );
	Output.vTangent  = mul( Input.vTangent,     (float3x3)g_mWorld );
	Output.vTexcoord = Input.vTexcoord;
	
	return Output;
}

