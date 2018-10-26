//TEST:COMPARE_HLSL:-no-mangle -profile ps_4_0 -entry PSMain

#ifndef __SLANG__
#define cbPerFrame cbPerFrame_0
#define g_vLightDir g_vLightDir_0
#define g_fAmbient g_fAmbient_0
#define g_samLinear g_samLinear_0
#define g_txDiffuse g_txDiffuse_0
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
	float4		g_vObjectColor			;//SLANG: : packoffset( c0 );
};

cbuffer cbPerFrame : register( b1 )
{
	float3		g_vLightDir				;//SLANG: : packoffset( c0 );
	float		g_fAmbient				;//SLANG: : packoffset( c0.w );
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

