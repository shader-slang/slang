// Check that annotated core-module resource intrinsics remove undefined accesses and retain
// their defined controls, including status outputs, comparison sampling, storage images and
// typed buffers.
//TEST:SIMPLE(filecheck=WRITE):-target spirv -entry writeTypedBuffer -stage compute
//TEST:SIMPLE(filecheck=WRITE):-target spirv -entry writeTypedBuffer -stage compute -O0
//TEST:SIMPLE(filecheck=WRITE):-target spirv -entry writeMultisampleImage -stage compute
//TEST:SIMPLE(filecheck=WRITE):-target spirv -entry writeMultisampleImage -stage compute -O0
//TEST:SIMPLE(filecheck=READ):-target spirv -entry readStorageImage -stage compute
//TEST:SIMPLE(filecheck=READ):-target spirv -entry readStorageImage -stage compute -O0
//TEST:SIMPLE(filecheck=DIMENSIONS):-target spirv -entry queryTypedBuffer -stage compute
//TEST:SIMPLE(filecheck=DIMENSIONS):-target spirv -entry queryTypedBuffer -stage compute -O0
//TEST:SIMPLE(filecheck=FETCH):-target spirv -entry readTypedBuffer -stage compute
//TEST:SIMPLE(filecheck=FETCH):-target spirv -entry readTypedBuffer -stage compute -O0
//TEST:SIMPLE(filecheck=READ):-target spirv -entry readWritableTypedBuffer -stage compute
//TEST:SIMPLE(filecheck=READ):-target spirv -entry readWritableTypedBuffer -stage compute -O0
//TEST:SIMPLE(filecheck=SPARSE_SAMPLE):-target spirv -entry sampleSparse -stage compute
//TEST:SIMPLE(filecheck=SPARSE_SAMPLE):-target spirv -entry sampleSparse -stage compute -O0
//TEST:SIMPLE(filecheck=SPARSE_FETCH):-target spirv -entry fetchSparse -stage compute
//TEST:SIMPLE(filecheck=SPARSE_FETCH):-target spirv -entry fetchSparse -stage compute -O0
//TEST:SIMPLE(filecheck=SPARSE_CMP):-target spirv -entry compareSparse -stage compute
//TEST:SIMPLE(filecheck=SPARSE_CMP):-target spirv -entry compareSparse -stage compute -O0
//TEST:SIMPLE(filecheck=COMPARE):-target spirv -entry compareImplicit -stage fragment
//TEST:SIMPLE(filecheck=COMPARE):-target spirv -entry compareImplicit -stage fragment -O0
//TEST:SIMPLE(filecheck=GATHER):-target spirv -entry gatherComparison -stage compute
//TEST:SIMPLE(filecheck=GATHER):-target spirv -entry gatherComparison -stage compute -O0

// WRITE: OpImageWrite
// WRITE-NOT: OpImageWrite
// READ: OpImageRead
// READ-NOT: OpImageRead
// DIMENSIONS: OpImageQuerySize
// DIMENSIONS-NOT: OpImageQuerySize
// FETCH: OpImageFetch
// FETCH-NOT: OpImageFetch
// SPARSE_SAMPLE: OpImageSparseSampleExplicitLod
// SPARSE_SAMPLE-NOT: OpImageSparseSampleExplicitLod
// SPARSE_FETCH: OpImageSparseFetch
// SPARSE_FETCH-NOT: OpImageSparseFetch
// SPARSE_CMP: OpImageSparseSampleDrefExplicitLod
// SPARSE_CMP-NOT: OpImageSparseSampleDrefExplicitLod
// COMPARE: OpImageSampleDrefImplicitLod
// COMPARE-NOT: OpImageSampleDrefImplicitLod
// GATHER: OpImageDrefGather
// GATHER-NOT: OpImageDrefGather

RWBuffer<float4> typedOutput;
Buffer<float4> typedInput;
RWTexture2D<float4> storageImage;
RWTexture2DMS<float4> multisampleImage;
Texture2D<float4> texture;
Texture2D<float> shadowTexture;
SamplerState sampler;
SamplerComparisonState comparisonSampler;
RWStructuredBuffer<float4> outputBuffer;
cbuffer Settings
{
    bool useUndefined;
};

[numthreads(1, 1, 1)]
void writeTypedBuffer()
{
    RWBuffer<float4> missing;
    if (useUndefined)
        missing[0] = float4(1);
    typedOutput[0] = float4(2);
}

[numthreads(1, 1, 1)]
void writeMultisampleImage()
{
    RWTexture2DMS<float4> missing;
    if (useUndefined)
        missing[uint2(0), 0] = float4(1);
    multisampleImage[uint2(0), 0] = float4(2);
}

[numthreads(1, 1, 1)]
void readStorageImage()
{
    RWTexture2D<float4> missing;
    float4 value = storageImage[uint2(0)];
    if (useUndefined)
        value = missing[uint2(0)];
    outputBuffer[0] = value;
}

[numthreads(1, 1, 1)]
void queryTypedBuffer()
{
    Buffer<float4> missing;
    uint count;
    typedInput.GetDimensions(count);
    if (useUndefined)
        missing.GetDimensions(count);
    outputBuffer[0] = float4(count);
}

[numthreads(1, 1, 1)]
void readTypedBuffer()
{
    Buffer<float4> missing;
    float4 value = typedInput.Load(0);
    if (useUndefined)
        value = missing.Load(0);
    outputBuffer[0] = value;
}

[numthreads(1, 1, 1)]
void readWritableTypedBuffer()
{
    RWBuffer<float4> missing;
    float4 value = typedOutput.Load(0);
    if (useUndefined)
        value = missing.Load(0);
    outputBuffer[0] = value;
}

[numthreads(1, 1, 1)]
void sampleSparse()
{
    SamplerState missing;
    uint status;
    float4 value = texture.SampleLevel(sampler, float2(0), 0, int2(0), status);
    if (useUndefined)
        value = texture.SampleLevel(missing, float2(0), 0, int2(0), status);
    outputBuffer[0] = value;
    outputBuffer[1] = float4(status);
}

[numthreads(1, 1, 1)]
void fetchSparse()
{
    Texture2D<float4> missing;
    uint status;
    float4 value = texture.Load(int3(0), int2(0), status);
    if (useUndefined)
        value = missing.Load(int3(0), int2(0), status);
    outputBuffer[0] = value;
    outputBuffer[1] = float4(status);
}

[numthreads(1, 1, 1)]
void compareSparse()
{
    SamplerComparisonState missing;
    uint status;
    float value = shadowTexture.SampleCmpLevelZero(comparisonSampler, float2(0), 0, int2(0), status);
    if (useUndefined)
        value = shadowTexture.SampleCmpLevelZero(missing, float2(0), 0, int2(0), status);
    outputBuffer[0] = float4(value);
    outputBuffer[1] = float4(status);
}

float compareImplicit() : SV_Target
{
    SamplerComparisonState missing;
    float value = shadowTexture.SampleCmp(comparisonSampler, float2(0), 0);
    if (useUndefined)
        value = shadowTexture.SampleCmp(missing, float2(0), 0);
    return value;
}

[numthreads(1, 1, 1)]
void gatherComparison()
{
    SamplerComparisonState missing;
    float4 value = shadowTexture.GatherCmpRed(comparisonSampler, float2(0), 0);
    if (useUndefined)
        value = shadowTexture.GatherCmpRed(missing, float2(0), 0);
    outputBuffer[0] = value;
}
