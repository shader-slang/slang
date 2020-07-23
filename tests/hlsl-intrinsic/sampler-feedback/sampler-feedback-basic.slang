//TEST:CROSS_COMPILE: -entry main -stage fragment -profile sm_6_5 -target dxil-assembly

FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> g_feedbackTexture;
FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIP_REGION_USED> g_feedbackTextureArray;

float4 main() : SV_Target
{
    
    return float4(1, 2, 3, 4);
}
