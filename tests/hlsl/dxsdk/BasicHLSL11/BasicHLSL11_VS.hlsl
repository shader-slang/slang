//TEST:COMPARE_HLSL: -target dxbc-assembly -profile vs_4_0 -entry VSMain

#ifndef __SLANG__
#define cbPerObject _SV032SLANG_parameterGroup_cbPerObject
#define g_mWorldViewProjection _SV032SLANG_ParameterGroup_cbPerObject22g_mWorldViewProjection
#define g_mWorld _SV032SLANG_ParameterGroup_cbPerObject8g_mWorld
#endif

//--------------------------------------------------------------------------------------
// File: BasicHLSL11_VS.hlsl
//
// The vertex shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	matrix		g_mWorldViewProjection	: packoffset( c0 );
	matrix		g_mWorld				: packoffset( c4 );
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
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition = mul( Input.vPosition, g_mWorldViewProjection );
	Output.vNormal = mul( Input.vNormal, (float3x3)g_mWorld );
	Output.vTexcoord = Input.vTexcoord;
	
	return Output;
}

