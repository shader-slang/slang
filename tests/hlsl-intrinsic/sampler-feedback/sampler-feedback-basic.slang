//TEST:REFLECTION: -entry main -stage fragment -profile sm_6_5 -target hlsl
//DISABLE_TEST:CROSS_COMPILE: -entry main -stage fragment -profile sm_6_5 -target dxil-assembly

FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> feedbackMinMip;
FeedbackTexture2D<SAMPLER_FEEDBACK_MIP_REGION_USED> feedbackMipRegionUsed;
FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIN_MIP> feedbackMinMipArray;
FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIP_REGION_USED> feebackMipRegionUsedArray;

Texture2D<float> tex2D;
Texture2D<float4> tex2D_float4;
Texture2DArray<float> tex2DArray;
SamplerState samp;


float4 main() : SV_Target
{
    float2 coords2D = float2(1, 2);
    float3 coords2DArray = float3(1, 2, 3);
    float clamp = 4;
    float bias = 0.5F;
    float lod = 6;
    float2 ddx = float2(1.0F / 32, 2.0F / 32);
    float2 ddy = float2(3.0F / 32, 4.0F / 32);
    
    float idx = 0;
    
    //feedbackMinMip.WriteSamplerFeedback(tex2D);
    
    //feedbackMinMip.WriteSamplerFeedback(tex2D, samp, coords2D + (10 * idx++), clamp);
    
    return float4(1, 2, 3, 4);
}
