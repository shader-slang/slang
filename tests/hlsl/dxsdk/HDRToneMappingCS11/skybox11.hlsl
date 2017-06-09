//TEST_IGNORE_FILE: Currently failing due to Spire compiler issues.
//TEST:COMPARE_HLSL: -target dxbc-assembly -profile vs_4_0 -entry SkyboxVS -profile ps_4_0 -entry SkyboxPS
//-----------------------------------------------------------------------------
// File: SkyBox11.hlsl
//
// Desc: 
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

cbuffer cbPerObject : register( b0 )
{
    row_major matrix    g_mWorldViewProjection	: packoffset( c0 );
}

TextureCube	g_EnvironmentTexture : register( t0 );
SamplerState g_sam : register( s0 );

struct SkyboxVS_Input
{
    float4 Pos : POSITION;
};

struct SkyboxVS_Output
{
    float4 Pos : SV_POSITION;
    float3 Tex : TEXCOORD0;
};

SkyboxVS_Output SkyboxVS( SkyboxVS_Input Input )
{
    SkyboxVS_Output Output;
    
    Output.Pos = Input.Pos;
    Output.Tex = normalize( mul(Input.Pos, g_mWorldViewProjection) );
    
    return Output;
}

float4 SkyboxPS( SkyboxVS_Output Input ) : SV_TARGET
{
    float4 color = g_EnvironmentTexture.Sample( g_sam, Input.Tex );
    return color;
}
