//TEST:COMPARE_HLSL:-no-mangle -target dxbc-assembly -profile ps_4_0 -entry PSMain

#ifndef __SLANG__
#define cbPerFrame _SV031SLANG_parameterGroup_cbPerFrame
#define g_vLightDir _SV031SLANG_ParameterGroup_cbPerFrame11g_vLightDir
#define g_fAmbient _SV031SLANG_ParameterGroup_cbPerFrame10g_fAmbient
#define g_samLinear _SV011g_samLinear
#define g_txDiffuse _SV011g_txDiffuse
#endif

//--------------------------------------------------------------------------------------
// File: BasicHLSL11_PS.hlsl
//
// The pixel shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	float4		g_vObjectColor			: packoffset( c0 );
};

cbuffer cbPerFrame : register( b1 )
{
	float3		g_vLightDir				: packoffset( c0 );
	float		g_fAmbient				: packoffset( c0.w );
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D	g_txDiffuse : register( t0 );
SamplerState g_samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	float4 vDiffuse = g_txDiffuse.Sample( g_samLinear, Input.vTexcoord );
	
	float fLighting = saturate( dot( g_vLightDir, Input.vNormal ) );
	fLighting = max( fLighting, g_fAmbient );
	
	return vDiffuse * fLighting;
}

