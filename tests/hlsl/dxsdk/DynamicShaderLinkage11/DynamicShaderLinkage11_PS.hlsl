//TEST_IGNORE_FILE: Currently failing due to Slang compiler issues.
//TEST:COMPARE_HLSL: -target dxbc-assembly -profile ps_4_0 -entry PSMain
//--------------------------------------------------------------------------------------
// File: DynamicShaderLinkage11.psh
//
// The pixel shader header file for the DynamicShaderLinkage11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Header Includes
//--------------------------------------------------------------------------------------
#include "DynamicShaderLinkage11_PSBuffers.h"

// Defines for default static permutated setting
#if defined( STATIC_PERMUTE ) 
   #define HEMI_AMBIENT //CONST_AMBIENT //HEMI_AMBIENT
   #define TEXTURE_ENABLE
   #define SPECULAR_ENABLE
#endif

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float4 vPosition	: SV_POSITION;
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
	float4 vMatrix	: TEXCOORD1;	
};

//--------------------------------------------------------------------------------------
// Abstract Interface Instances for dyamic linkage / permutation
//--------------------------------------------------------------------------------------
#if !defined( STATIC_PERMUTE ) 
    iBaseLight     g_abstractAmbientLighting;
    iBaseLight     g_abstractDirectLighting;
    iBaseLight     g_abstractEnvironmentLighting;
    iBaseMaterial  g_abstractMaterial;
#else
//--------------------------------------------------------------------------------------
// Concrete Instances for STATIC_PERMUTE - static permutation
//--------------------------------------------------------------------------------------
    #if defined( HEMI_AMBIENT ) 
        #define g_abstractAmbientLighting g_hemiAmbientLight
    #else  
        // CONST_AMBIENT
        #define g_abstractAmbientLighting g_ambientLight
    #endif
    #define g_abstractDirectLighting g_directionalLight
    #define g_abstractEnvironmentLighting g_environmentLight
    #if defined( TEXTURE_ENABLE )
        #define g_abstractMaterial g_plasticTexturedMaterial
    #else    
        #define g_abstractMaterial g_plasticMaterial
    #endif
#endif

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{   
   // Compute the Ambient term
   float3   Ambient = (float3)0.0f;	
   Ambient = g_abstractMaterial.GetAmbientColor( Input.vTexcoord ) * g_abstractAmbientLighting.IlluminateAmbient( Input.vNormal );

   // Accumulate the Diffuse contribution  
   float3   Diffuse = (float3)0.0f;  
   
   Diffuse += g_abstractMaterial.GetDiffuseColor( Input.vTexcoord ) * g_abstractDirectLighting.IlluminateDiffuse( Input.vNormal );

   // Compute the Specular contribution
   float3   Specular = (float3)0.0f;   
   Specular += g_abstractDirectLighting.IlluminateSpecular( Input.vNormal, g_abstractMaterial.GetSpecularPower() );
   Specular += g_abstractEnvironmentLighting.IlluminateSpecular( Input.vNormal, g_abstractMaterial.GetSpecularPower() );
     
   // Accumulate the lighting with saturation
   float3 Lighting = saturate( Ambient + Diffuse + Specular );
     
   return float4(Lighting,1.0f); 
}
