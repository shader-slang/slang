//TEST_IGNORE_FILE: Currently failing due to Spire compiler issues.
//TEST:COMPARE_HLSL: -target dxbc-assembly -profile ps_4_0 -entry PSMain
//--------------------------------------------------------------------------------------
// File: MultithreadedRendering11_PS.hlsl
//
// The pixel shader file for the MultithreadedRendering11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// Various debug options
//#define NO_DIFFUSE_MAP
//#define NO_NORMAL_MAP
//#define NO_AMBIENT
//#define NO_DYNAMIC_LIGHTING
//#define NO_SHADOW_MAP

#define SHADOW_DEPTH_BIAS 0.0005f

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
static const int g_iNumLights = 4;
static const int g_iNumShadows = 1; // by convention, the first n lights cast shadows

cbuffer cbPerObject : register( b0 )
{
	float4		g_vObjectColor			: packoffset( c0 );
};

cbuffer cbPerLight : register( b1 )
{
    struct LightDataStruct
    {
	    matrix		m_mLightViewProj;
	    float4		m_vLightPos;
	    float4		m_vLightDir;
	    float4		m_vLightColor;
	    float4		m_vFalloffs;    // x = dist end, y = dist range, z = cos angle end, w = cos range
	} g_LightData[g_iNumLights]         : packoffset( c0 );
};

cbuffer cbPerScene : register( b2 )
{
	float4		g_vMirrorPlane			: packoffset( c0 );
	float4  	g_vAmbientColor			: packoffset( c1 );
	float4		g_vTintColor			: packoffset( c2 );
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D	        g_txDiffuse                : register( t0 );
Texture2D	        g_txNormal                 : register( t1 );
Texture2D           g_txShadow[g_iNumShadows]  : register( t2 );

SamplerState        g_samPointClamp : register( s0 );
SamplerState        g_samLinearWrap : register( s1 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float3 vNormal		: NORMAL;
	float3 vTangent		: TANGENT;
	float2 vTexcoord	: TEXCOORD0;
	float4 vPosWorld	: TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Sample normal map, convert to signed, apply tangent-to-world space transform
//--------------------------------------------------------------------------------------
float3 CalcPerPixelNormal( float2 vTexcoord, float3 vVertNormal, float3 vVertTangent )
{
	// Compute tangent frame
	vVertNormal =   normalize( vVertNormal );	
	vVertTangent =  normalize( vVertTangent );	
	float3 vVertBinormal = normalize( cross( vVertTangent, vVertNormal ) );	
	float3x3 mTangentSpaceToWorldSpace = float3x3( vVertTangent, vVertBinormal, vVertNormal ); 
	
	// Compute per-pixel normal
	float3 vBumpNormal = g_txNormal.Sample( g_samLinearWrap, vTexcoord );
	vBumpNormal = 2.0f * vBumpNormal - 1.0f;
	
	return mul( vBumpNormal, mTangentSpaceToWorldSpace );
}

//--------------------------------------------------------------------------------------
// Test how much pixel is in shadow, using 2x2 percentage-closer filtering
//--------------------------------------------------------------------------------------
float4 CalcUnshadowedAmountPCF2x2( int iShadow, float4 vPosWorld )
{
    matrix mLightViewProj = g_LightData[iShadow].m_mLightViewProj;
    Texture2D txShadow =    g_txShadow[iShadow]; 

    // Compute pixel position in light space
    float4 vLightSpacePos = mul( vPosWorld, mLightViewProj ); 
    vLightSpacePos.xyz /= vLightSpacePos.w;
    
    // Translate from surface coords to texture coords
    // Could fold these into the matrix
    float2 vShadowTexCoord = 0.5f * vLightSpacePos + 0.5f;
    vShadowTexCoord.y = 1.0f - vShadowTexCoord.y;
    
    // Depth bias to avoid pixel self-shadowing
    float vLightSpaceDepth = vLightSpacePos.z - SHADOW_DEPTH_BIAS;
    
    // Find sub-pixel weights
    float2 vShadowMapDims = float2( 2048.0f, 2048.0f ); // need to keep in sync with .cpp file
    float4 vSubPixelCoords;
    vSubPixelCoords.xy = frac( vShadowMapDims * vShadowTexCoord );
    vSubPixelCoords.zw = 1.0f - vSubPixelCoords;
    float4 vBilinearWeights = vSubPixelCoords.zxzx * vSubPixelCoords.wwyy;

    // 2x2 percentage closer filtering
    float2 vTexelUnits = 1.0f / vShadowMapDims;
    float4 vShadowDepths;
    vShadowDepths.x = txShadow.Sample( g_samPointClamp, vShadowTexCoord );
    vShadowDepths.y = txShadow.Sample( g_samPointClamp, vShadowTexCoord + float2( vTexelUnits.x, 0.0f ) );
    vShadowDepths.z = txShadow.Sample( g_samPointClamp, vShadowTexCoord + float2( 0.0f, vTexelUnits.y ) );
    vShadowDepths.w = txShadow.Sample( g_samPointClamp, vShadowTexCoord + vTexelUnits );
    
    // What weighted fraction of the 4 samples are nearer to the light than this pixel?
    float4 vShadowTests = ( vShadowDepths >= vLightSpaceDepth ) ? 1.0f : 0.0f;
    return dot( vBilinearWeights, vShadowTests );
}

//--------------------------------------------------------------------------------------
// Diffuse lighting calculation, with angle and distance falloff
//--------------------------------------------------------------------------------------
float4 CalcLightingColor( int iLight, float3 vPosWorld, float3 vPerPixelNormal )
{
    float3 vLightPos =      g_LightData[iLight].m_vLightPos.xyz; 
    float3 vLightDir =      g_LightData[iLight].m_vLightDir.xyz;
    float4 vLightColor =    g_LightData[iLight].m_vLightColor; 
    float4 vFalloffs =      g_LightData[iLight].m_vFalloffs; 
    
    float3 vLightToPixelUnNormalized = vPosWorld - vLightPos;
    
    // Dist falloff = 0 at vFalloffs.x, 1 at vFalloffs.x - vFalloffs.y
    float fDist = length( vLightToPixelUnNormalized );
    float fDistFalloff = saturate( ( vFalloffs.x - fDist ) / vFalloffs.y );
    
    // Normalize from here on
    float3 vLightToPixelNormalized = vLightToPixelUnNormalized / fDist;
    
    // Angle falloff = 0 at vFalloffs.z, 1 at vFalloffs.z - vFalloffs.w
    float fCosAngle = dot( vLightToPixelNormalized, vLightDir );
    float fAngleFalloff = saturate( ( fCosAngle - vFalloffs.z ) / vFalloffs.w );
    
    // Diffuse contribution
    float fNDotL = saturate( -dot( vLightToPixelNormalized, vPerPixelNormal ) );
    
	return vLightColor * fNDotL * fDistFalloff * fAngleFalloff;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
    // Manual clip test, so that objects which are behind the mirror 
    // don't show up in the mirror.
    clip( dot( g_vMirrorPlane.xyz, Input.vPosWorld.xyz ) + g_vMirrorPlane.w );

#ifdef NO_DIFFUSE_MAP
	float4 vDiffuse = 0.5f;
#else   // #ifdef NO_DIFFUSE_MAP
	float4 vDiffuse = g_txDiffuse.Sample( g_samLinearWrap, Input.vTexcoord );
#endif  // #ifdef NO_DIFFUSE_MAP #else
	
	// Compute per-pixel normal
#ifdef NO_NORMAL_MAP
	float3 vPerPixelNormal = Input.vNormal;
#else   // #ifdef NO_NORMAL_MAP
	float3 vPerPixelNormal = CalcPerPixelNormal( Input.vTexcoord, Input.vNormal, Input.vTangent );
#endif  // #ifdef NO_NORMAL_MAP #else

    // Compute lighting contribution
#ifdef NO_AMBIENT
	float4 vTotalLightingColor = 0.0f;
#else   // #ifdef NO_AMBIENT
	float4 vTotalLightingColor = g_vAmbientColor;
#endif  // #ifdef NO_AMBIENT #else

#ifndef NO_DYNAMIC_LIGHTING
	for ( int iLight = 0; iLight < g_iNumLights; ++iLight )
	{
        float4 vLightingColor = CalcLightingColor( iLight, Input.vPosWorld, vPerPixelNormal );
#ifndef NO_SHADOW_MAP
	    if ( iLight < g_iNumShadows && any( vLightingColor.xyz ) > 0.0f ) // Don't bother checking shadow map if the pixel is unlit
	    {
            vLightingColor *= CalcUnshadowedAmountPCF2x2( iLight, Input.vPosWorld );
	    }
#endif  // #ifndef NO_SHADOW_MAP
	    vTotalLightingColor += vLightingColor;
	}
#endif  // #ifndef NO_DYNAMIC_LIGHTING

	return vDiffuse * g_vTintColor * g_vObjectColor * vTotalLightingColor;
}
