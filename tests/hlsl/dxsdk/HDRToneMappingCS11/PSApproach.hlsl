//TEST_IGNORE_FILE: Currently failing due to Spire compiler issues.
//TEST:COMPARE_HLSL: -target dxbc-assembly -profile ps_4_0 -entry DownScale2x2_Lum -entry DownScale3x3 -entry FinalPass -entry DownScale3x3_BrightPass -entry Bloom
//--------------------------------------------------------------------------------------
// File: PSApproach.hlsl
//
// The PSs for doing post-processing, used in PS path of 
// HDRToneMappingCS11 sample
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
static const float4 LUM_VECTOR = float4(.299, .587, .114, 0);
static const float  MIDDLE_GRAY = 0.72f;
static const float  LUM_WHITE = 1.5f;
static const float  BRIGHT_THRESHOLD = 0.5f;

SamplerState PointSampler : register (s0);
SamplerState LinearSampler : register (s1);

struct QuadVS_Output
{
    float4 Pos : SV_POSITION;              
    float2 Tex : TEXCOORD0;
};

Texture2D s0 : register(t0);
Texture2D s1 : register(t1);
Texture2D s2 : register(t2);

float4 DownScale2x2_Lum ( QuadVS_Output Input ) : SV_TARGET
{    
    float4 vColor = 0.0f;
    float  fAvg = 0.0f;
    
    for( int y = -1; y < 1; y++ )
    {
        for( int x = -1; x < 1; x++ )
        {
            // Compute the sum of color values
            vColor = s0.Sample( PointSampler, Input.Tex, int2(x,y) );                       
                
            fAvg += dot( vColor, LUM_VECTOR );
        }
    }
    
    fAvg /= 4;
    
    return float4(fAvg, fAvg, fAvg, 1.0f);
}

float4 DownScale3x3( QuadVS_Output Input ) : SV_TARGET
{
    float fAvg = 0.0f; 
    float4 vColor;
    
    for( int y = -1; y <= 1; y++ )
    {
        for( int x = -1; x <= 1; x++ )
        {
            // Compute the sum of color values
            vColor = s0.Sample( PointSampler, Input.Tex, int2(x,y) );
                        
            fAvg += vColor.r; 
        }
    }
    
    // Divide the sum to complete the average
    fAvg /= 9;
    
    return float4(fAvg, fAvg, fAvg, 1.0f);
}

float4 FinalPass( QuadVS_Output Input ) : SV_TARGET
{   
    //float4 vColor = 0;
    float4 vColor = s0.Sample( PointSampler, Input.Tex );
    float4 vLum = s1.Sample( PointSampler, float2(0,0) );
    float3 vBloom = s2.Sample( LinearSampler, Input.Tex );       
    
    // Tone mapping
    vColor.rgb *= MIDDLE_GRAY / (vLum.r + 0.001f);
    vColor.rgb *= (1.0f + vColor/LUM_WHITE);
    vColor.rgb /= (1.0f + vColor);
    
    vColor.rgb += 0.6f * vBloom;
    vColor.a = 1.0f;    
    
    return vColor;
}

float4 DownScale3x3_BrightPass( QuadVS_Output Input ) : SV_TARGET
{   
    float3 vColor = 0.0f;
    float4 vLum = s1.Sample( PointSampler, float2(0, 0) );
    float  fLum = vLum.r;

    vColor = s0.Sample( PointSampler, Input.Tex ).rgb;          
 
    // Bright pass and tone mapping
    vColor = max( 0.0f, vColor - BRIGHT_THRESHOLD );
    vColor *= MIDDLE_GRAY / (fLum + 0.001f);
    vColor *= (1.0f + vColor/LUM_WHITE);
    vColor /= (1.0f + vColor);
    
    return float4(vColor, 1.0f);
}

cbuffer cb0
{
    float2 g_avSampleOffsets[15];
    float4 g_avSampleWeights[15];
}

float4 Bloom( QuadVS_Output Input ) : SV_TARGET
{    
    float4 vSample = 0.0f;
    float4 vColor = 0.0f;
    float2 vSamplePosition;
    
    for( int iSample = 0; iSample < 15; iSample++ )
    {
        // Sample from adjacent points
        vSamplePosition = Input.Tex + g_avSampleOffsets[iSample];
        vColor = s0.Sample( PointSampler, vSamplePosition);
        
        vSample += g_avSampleWeights[iSample]*vColor;
    }
    
    return vSample;
}
