//--------------------------------------------------------------------------------------
// File: DynamicShaderLinkageFX11_LightPSH.hlsl
//
// The pixel shader light source module file for the DynamicShaderLinkageFX11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "DynamicShaderLinkageFX11_LightPSH.h"
#include "DynamicShaderLinkageFX11_MaterialPSH.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbPerFrame : register( b0 )
{
   cAmbientLight     g_ambientLight;
   cHemiAmbientLight g_hemiAmbientLight;
   cDirectionalLight g_directionalLight;
   cEnvironmentLight g_environmentLight;
   float4            g_vEyeDir;   
};

cbuffer cbPerPrimitive : register( b1 )
{
   cPlasticMaterial              g_plasticMaterial;
   cPlasticTexturedMaterial      g_plasticTexturedMaterial;
   cPlasticLightingOnlyMaterial  g_plasticLightingOnlyMaterial;
   cRoughMaterial                g_roughMaterial;
   cRoughTexturedMaterial        g_roughTexturedMaterial;
   cRoughLightingOnlyMaterial    g_roughLightingOnlyMaterial;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D      g_txDiffuse : register( t0 );
Texture2D      g_txNormalMap : register( t1 );
TextureCube    g_txEnvironmentMap : register( t2 );

SamplerState   g_samLinear : register( s0 )
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
};

//--------------------------------------------------------------------------------------
// Rasterization State
//--------------------------------------------------------------------------------------
uint g_fillMode = 0;
   
RasterizerState g_rasterizerState[2]
{
{
    FillMode = SOLID;
    MultisampleEnable = true;
},
{
    FillMode = WIREFRAME;
    MultisampleEnable = true;
}
};

//--------------------------------------------------------------------------------------
// Lighting Class Methods
//--------------------------------------------------------------------------------------
// Ambient Lighting Class Methods
float3 cAmbientLight::IlluminateAmbient(float3 vNormal)
{ 
   return m_vLightColor * m_bEnable;
}

float3 cHemiAmbientLight::IlluminateAmbient(float3 vNormal)
{ 
   float thetha = (dot( vNormal, m_vDirUp.xyz ) + 1.0f) / 2.0f;
 
   return  lerp( m_vGroundColor.xyz, m_vLightColor, thetha) * m_bEnable;
}

// Directional Light class
float3 cDirectionalLight::IlluminateDiffuse( float3 vNormal ) 
{
   float lambert = saturate(dot( vNormal, m_vLightDir.xyz ));
   return ((float3)lambert * m_vLightColor * m_bEnable);
}

float3 cDirectionalLight::IlluminateSpecular( float3 vNormal, int specularPower ) 
{       
   float3 H = -normalize(g_vEyeDir.xyz) + m_vLightDir.xyz;
   float3 halfAngle = normalize( H );
   float specular = pow( max(0,dot( halfAngle, normalize(vNormal) )), specularPower );          

        return ((float3)specular * m_vLightColor * m_bEnable); 
}

// Omni Light Class
float3 cOmniLight::IlluminateDiffuse( float3 vNormal ) 
{
   return (float3)0.0f; // TO DO!
}

// Environment Lighting
float3 cEnvironmentLight::IlluminateSpecular( float3 vNormal, int specularPower ) 
{         
   // compute reflection vector taking into account a cheap fresnel falloff;
   float3 N = normalize(vNormal); 
   float3 E = normalize(g_vEyeDir.xyz);
   float3 R = reflect( E, N ); 
   float fresnel = 1 - dot( -E, N );    
   fresnel = (fresnel * fresnel * fresnel );

   float3 specular = g_txEnvironmentMap.Sample( g_samLinear, R ).xyz * fresnel;

   return (specular * (float3)m_bEnable); 
//   return ((float3)fresnel); 

}

//--------------------------------------------------------------------------------------
// Material Class Methods
//--------------------------------------------------------------------------------------
// Plastic Material Methods
float3 cPlasticTexturedMaterial::GetAmbientColor(float2 vTexcoord)
{ 
   float4 vDiffuse = (float4)1.0f;
   vDiffuse = g_txDiffuse.Sample( g_samLinear, vTexcoord );  
   return m_vColor * vDiffuse.xyz;
}
   
float3 cPlasticTexturedMaterial::GetDiffuseColor(float2 vTexcoord)
{ 
   float4 vDiffuse = (float4)1.0f;
   vDiffuse = g_txDiffuse.Sample( g_samLinear, vTexcoord );  
   return m_vColor * vDiffuse.xyz;
}

// Rough Material Methods
float3 cRoughTexturedMaterial::GetAmbientColor(float2 vTexcoord)
{ 
   float4 vDiffuse = (float4)1.0f;
   vDiffuse = g_txDiffuse.Sample( g_samLinear, vTexcoord );  
   return m_vColor * vDiffuse.xyz;
}
   
float3 cRoughTexturedMaterial::GetDiffuseColor(float2 vTexcoord)
{ 
   float4 vDiffuse = (float4)1.0f;
   vDiffuse = g_txDiffuse.Sample( g_samLinear, vTexcoord );  
   return m_vColor * vDiffuse.xyz;
}
