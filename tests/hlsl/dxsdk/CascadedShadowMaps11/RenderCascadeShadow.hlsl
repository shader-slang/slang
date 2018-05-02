//TEST:COMPARE_HLSL: -target dxbc-assembly -profile vs_4_0 -entry VSMain -entry VSMainPancake

#ifndef __SLANG__
#define cbPerObject _SV032SLANG_parameterGroup_cbPerObject
#define g_mWorldViewProjection _SV032SLANG_ParameterGroup_cbPerObject22g_mWorldViewProjection
#endif

//--------------------------------------------------------------------------------------
// File: RenderCascadeShadow.hlsl
//
// The shader file for the RenderCascadeScene sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
    matrix        g_mWorldViewProjection    ;//SLANG: : packoffset( c0 );
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 vPosition    : POSITION;
};

struct VS_OUTPUT
{
    float4 vPosition    : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
    VS_OUTPUT Output;
    
    // There is nothing special here, just transform and write out the depth.
    Output.vPosition = mul( Input.vPosition, g_mWorldViewProjection );

    return Output;
}


VS_OUTPUT VSMainPancake( VS_INPUT Input )
{
    VS_OUTPUT Output;
    // after transform move clipped geometry to near plane
    Output.vPosition = mul( Input.vPosition, g_mWorldViewProjection );
	//Output.vPosition.z = max( Output.vPosition.z, 0.0f );
    return Output;
}