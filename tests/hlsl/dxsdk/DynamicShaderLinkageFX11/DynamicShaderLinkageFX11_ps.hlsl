//TEST_IGNORE_FILE:
//--------------------------------------------------------------------------------------
// File: DynamicShaderLinkageFX11.psh
//
// The pixel shader header file for the DynamicShaderLinkageFX11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Header Includes
//--------------------------------------------------------------------------------------
#include "DynamicShaderLinkageFX11_PSBuffers.h"

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
    float4 vPosition    : SV_POSITION;
    float3 vNormal      : NORMAL;
    float2 vTexcoord    : TEXCOORD0;
    float4 vMatrix      : TEXCOORD1;    
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

// This pixel shader uses several interfaces during its
// work.  We show three different ways of providing interface
// bindings for the PS and those have two different
// entry points so we've separated the base PS code
// into a worker routine that's called by the entry
// points.  Normally only one technique would be used
// and this layering of entry point and worker would
// not be necessary.
float4 PSMainWorker( iBaseLight ambientLighting,
                     iBaseLight directLighting,
                     iBaseLight environmentLighting,
                     iBaseMaterial material,
                     PS_INPUT Input )
{   
   // Compute the Ambient term
   float3   Ambient = (float3)0.0f; 
   Ambient = material.GetAmbientColor( Input.vTexcoord ) * ambientLighting.IlluminateAmbient( Input.vNormal );

   // Accumulate the Diffuse contribution  
   float3   Diffuse = (float3)0.0f;  
   
   Diffuse += material.GetDiffuseColor( Input.vTexcoord ) * directLighting.IlluminateDiffuse( Input.vNormal );

   // Compute the Specular contribution
   float3   Specular = (float3)0.0f;   
   Specular += directLighting.IlluminateSpecular( Input.vNormal, material.GetSpecularPower() );
   Specular += environmentLighting.IlluminateSpecular( Input.vNormal, material.GetSpecularPower() );
     
   // Accumulate the lighting with saturation
   float3 Lighting = saturate( Ambient + Diffuse + Specular);

   return float4(Lighting,1.0f);
}

// One way to provide bindings for shaders in Effects 11 is
// to use uniform interface parameters.  As with non-interface
// uniform parameters you must specify a value for these
// parameters in your CompileShader invocations in the effect.
// You can provide concrete class instances if you want
// to statically specialize your shaders, such as for targets
// that don't support abstract interfaces; or you can provide
// other interfaces that you bind using effect variables.
// Both are shown in this sample's technique passes.
float4 PSMainUniform( uniform iBaseLight ambientLighting,
                      uniform iBaseLight directLighting,
                      uniform iBaseLight environmentLighting,
                      uniform iBaseMaterial material,
                      PS_INPUT Input ) : SV_Target
{
    return PSMainWorker(ambientLighting,
                        directLighting,
                        environmentLighting,
                        material,
                        Input);
}

// Another way to use Effects 11 with interfaces is
// to have non-uniform parameters, which then are
// bound with a BindInterfaces in a technique pass.
// BindInterfaces gives concrete instances to use
// with a shader but does not do static specialization,
// it just saves information for the effect runtime
// to use when setting up the shader to run.
// This lets you share a single shader, compiled with
// interface usage, while still getting the convenience
// of declaring concrete bindings in the effect and
// not needed explicit binding in code via effect
// variable updates.  If you have many different
// variations it may be simpler to use bindings
// through effect variables, as then you don't
// need to list every possible binding set in your
// techniques.
float4 PSMainNonUniform( iBaseLight ambientLighting,
                         iBaseLight directLighting,
                         iBaseLight environmentLighting,
                         iBaseMaterial material,
                         PS_INPUT Input ) : SV_Target
{
    return PSMainWorker(ambientLighting,
                        directLighting,
                        environmentLighting,
                        material,
                        Input);
}
