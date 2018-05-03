//TEST:COMPARE_HLSL: -target dxbc-assembly -profile vs_4_0 -entry VSMain

#ifndef __SLANG__
#define cbPerObject cbPerObject_0
#define g_mWorldViewProjection g_mWorldViewProjection_0
#define g_mWorld g_mWorld_0
#endif

//--------------------------------------------------------------------------------------
// File: DynamicShaderLinkage11_VS.hlsl
//
// The vertex shader file for the DynamicShaderLinkage11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	float4x4		g_mWorldViewProjection	;//SLANG: : packoffset( c0 );
	float4x4		g_mWorld				;//SLANG: : packoffset( c4 );
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
	float3 vNormal		: NORMAL;
	float2 vTexcoord0	: TEXCOORD0;
	float4 vMatrix	    : TEXCOORD1; // DEBUG
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
// We aliased signed vectors as a unsigned format. 
// Need to recover signed values.  The values 1.0 and 2.0
// are slightly inaccurate here.
float3 R10G10B10A2_UNORM_TO_R32G32B32_FLOAT( in float3 vVec )
{
    vVec *= 2.0f;
    return vVec >= 1.0f ? ( vVec - 2.0f ) : vVec;
}

VS_OUTPUT VSMain( VS_INPUT Input )
{

	VS_OUTPUT   Output;
	float3      tmpNormal;
	
	Output.vPosition =  mul( Input.vPosition, g_mWorldViewProjection );
	
	// Expand compressed vectors
	tmpNormal = R10G10B10A2_UNORM_TO_R32G32B32_FLOAT( Input.vNormal );
	Output.vNormal = mul( tmpNormal, (float3x3)g_mWorld );
	
	Output.vTexcoord0 = Input.vTexcoord;

    Output.vMatrix = (float4)g_mWorld[0]; // DEBUG
	return Output;
}

