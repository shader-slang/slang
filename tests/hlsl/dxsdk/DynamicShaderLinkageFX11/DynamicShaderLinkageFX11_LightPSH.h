//--------------------------------------------------------------------------------------
// File: DynamicShaderLinkageFX11_LightPSH.h
//
// The pixel shader light header file for the DynamicShaderLinkageFX11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Interfaces
//--------------------------------------------------------------------------------------
interface iBaseLight
{
   float3 IlluminateAmbient(float3 vNormal);
   
   float3 IlluminateDiffuse(float3 vNormal);

   float3 IlluminateSpecular(float3 vNormal, int specularPower );
   
};

//--------------------------------------------------------------------------------------
// Classes
//--------------------------------------------------------------------------------------
class cAmbientLight : iBaseLight
{
   float3   m_vLightColor;     
   bool     m_bEnable;
   
   float3 IlluminateAmbient(float3 vNormal);
      
   float3 IlluminateDiffuse(float3 vNormal)
   { 
      return (float3)0;
   }

   float3 IlluminateSpecular(float3 vNormal, int specularPower )
   { 
      return (float3)0;
   }
};

class cHemiAmbientLight : cAmbientLight
{
   // inherited float4 m_vLightColor is the SkyColor
   float4   m_vGroundColor;
   float4   m_vDirUp;

   float3 IlluminateAmbient(float3 vNormal);
   
};

class cDirectionalLight : cAmbientLight
{
   // inherited float4 m_vLightColor is the LightColor
   float4 m_vLightDir;
   
   float3 IlluminateDiffuse( float3 vNormal );

   float3 IlluminateSpecular( float3 vNormal, int specularPower );

};

class cOmniLight : cAmbientLight
{
   float3   m_vLightPosition;
   float    radius;   
   
   float3 IlluminateDiffuse( float3 vNormal );
  
};

class cSpotLight : cAmbientLight
{
   float3   m_vLightPosition;
   float3   m_vLightDir;
};

class cEnvironmentLight : cAmbientLight
{
   float3  IlluminateSpecular( float3 vNormal, int specularPower );  
};
