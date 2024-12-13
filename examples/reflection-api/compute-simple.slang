// compute-simple.slang

struct ImageProcessingOptions
{
    float3 tintColor;
    float blurRadius;

    bool useLookupTable;
    StructuredBuffer<float4> lookupTable;
}

[shader("compute")]
[numthreads(8, 8)]
void processImage(
    uint3 threadID : SV_DispatchThreadID,
    uniform Texture2D inputImage,
    uniform RWTexture2D outputImage,
    uniform ImageProcessingOptions options)
{
    /* actual logic would go here */
}
