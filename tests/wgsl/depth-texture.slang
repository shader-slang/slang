//TEST:SIMPLE(filecheck=WGSL): -stage fragment -entry fragMain -target wgsl

// Tests all depth texture ops supported by WGSL
//
// In WGSL, depth texture shaders always use f32.


//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;


//TEST_INPUT: Texture2D(size=4, content = zero):name d2D
DepthTexture2D d2D;

//TEST_INPUT: Texture2D(size=4, content = zero):name d2DMS
DepthTexture2DMS d2DMS;

//TEST_INPUT: Texture2D(size=4, content = zero, arrayLength=2):name d2DArray
DepthTexture2DArray d2DArray;

//TEST_INPUT: TextureCube(size=4, content = zero):name dCube
DepthTextureCube dCube;

//TEST_INPUT: TextureCube(size=4, content = zero, arrayLength=2):name dCubeArray
DepthTextureCubeArray dCubeArray;


//TEST_INPUT: Sampler:name samplerState
SamplerState samplerState;

//TEST_INPUT: Sampler:name depthSampler
SamplerComparisonState depthSampler;


// All ops defined for textures in WGSL ( https://www.w3.org/TR/WGSL/#texture-builtin-functions )
//
// | Op                           | Depth texture types                | Tested by
// |------------------------------|------------------------------------|-------------------------------------------------
// | textureDimensions            | 2D, 2DArray, Cube, CubeArray, 2DMS | TEST_DepthTexture_getDimensions()
// | textureGather                | 2D, 2DArray, Cube, CubeArray       | texture-gather.slang: TEST_textureGather_depth()
// | textureGatherCompare         | 2D, 2DArray, Cube, CubeArray       | texture-gather.slang: TEST_textureGather_depth()
// | textureLoad                  | 2D, 2DArray, 2DMS                  | texture-load.slang: TEST_textureLoad_depth()
// | textureNumLayers             | 2DArray, CubeArray                 | TEST_DepthTexture_getDimensions()
// | textureNumLevels             | 2D, 2DArray, Cube, CubeArray       | TEST_DepthTexture_getMipMapLevels()
// | textureNumSamples            | 2DMS                               | TEST_DepthTexture_getDimensions()
// | textureSample                | 2D, 2DArray, Cube, CubeArray       | TEST_DepthTexture_textureSample()
// | textureSampleBias            | 2D, 2DArray, Cube, CubeArray       | TEST_DepthTexture_textureSample()
// | textureSampleCompare         | 2D, 2DArray, Cube, CubeArray       | TEST_DepthTexture_textureSampleCompare()
// | textureSampleCompareLevel    | 2D, 2DArray, Cube, CubeArray       | TEST_DepthTexture_textureSampleCompare()
// | textureSampleGrad            | None                               | N/A
// | textureSampleLevel           | 2D, 2DArray, Cube, CubeArray       | texture.slang: TEST_texture<T>()
// | textureSampleBaseClampToEdge | None                               | N/A
// | textureStore                 | None                               | N/A
//


bool TEST_DepthTexture_getDimensions()
{
    // WGSL-LABEL: TEST_DepthTexture_getDimensions

    uint width = 0, height = 0;
    uint samples = 0;
    uint layers = 0;
    bool voidResult = true;

    // WGSL: textureDimensions{{\(+}}d2D_
    d2D.GetDimensions(width, height);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);

    // WGSL: textureDimensions{{\(+}}d2DMS_
    // WGSL: textureNumSamples{{\(+}}d2DMS_
    d2DMS.GetDimensions(width, height, samples);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(1) == samples);

    // WGSL: textureDimensions{{\(+}}d2DArray_
    // WGSL: textureNumLayers{{\(+}}d2DArray_
    d2DArray.GetDimensions(width, height, layers);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(2) == layers);

    return voidResult;
}

bool TEST_DepthTexture_getMipMapLevels()
{
    // WGSL-LABEL: TEST_DepthTexture_getMipMapLevels

    uint width = 0, height = 0;
    uint samples = 0;
    uint layers = 0;
    uint levels = 0;
    bool voidResult = true;

    // WGSL: textureNumLevels{{\(+}}d2D_
    d2D.GetDimensions(0, width, height, levels);
    voidResult = voidResult && (uint(1) == levels);

    // WGSL: textureNumLevels{{\(+}}d2DArray_
    d2DArray.GetDimensions(0, width, height, layers, levels);
    voidResult = voidResult && (uint(1) == levels);

    // WGSL: textureNumLevels{{\(+}}dCube_
    dCube.GetDimensions(0, width, height, levels);
    voidResult = voidResult && (uint(1) == levels);

    // WGSL: textureNumLevels{{\(+}}dCubeArray_
    dCubeArray.GetDimensions(0, width, height, layers, levels);
    voidResult = voidResult && (uint(1) == levels);

    return voidResult;
}

bool TEST_DepthTexture_textureSample()
{
    // WGSL-LABEL: TEST_DepthTexture_textureSample

    bool result = true;
    float3 cubeDir = normalize(float3(0.3, 0.4, 0.5));
    float4 cubeDirAndArrayIndex = float4(normalize(float3(0.3, 0.4, 0.5)), 1);

    // WGSL-COUNT-2: textureSample{{\(+}}d2D_
    result = result
        && float(0.0f) == d2D.Sample(samplerState, float2(0.1f, 0.2f))             // base variant
        && float(0.0f) == d2D.Sample(samplerState, float2(0.1f, 0.2f), int2(0, 1)) // offset variant
        ;

    // WGSL-COUNT-2: textureSample{{\(+}}d2DArray_
    result = result
        && float(0.0f) == d2DArray.Sample(samplerState, float3(0.1f, 0.2f, 0.5f))             // base variant
        && float(0.0f) == d2DArray.Sample(samplerState, float3(0.1f, 0.2f, 0.5f), int2(0, 1)) // offset variant
        ;

    // WGSL-COUNT-1: textureSample{{\(+}}dCube_
    result = result
        && float(0.0f) == dCube.Sample(samplerState, cubeDir)             // base variant
        ;

    // WGSL-COUNT-1: textureSample{{\(+}}dCubeArray_
    result = result
        && float(0.0f) == dCubeArray.Sample(samplerState, cubeDirAndArrayIndex)             // base variant
        ;

    // WGSL-COUNT-2: textureSampleBias{{\(+}}d2D_
    result = result
        && float(0.0f) == d2D.SampleBias(samplerState, float2(0.1f, 0.2f), 0.9f)             // base variant
        && float(0.0f) == d2D.SampleBias(samplerState, float2(0.1f, 0.2f), 0.9f, int2(0, 1)) // offset variant
        ;

    // WGSL-COUNT-2: textureSampleBias{{\(+}}d2DArray_
    result = result
        && float(0.0f) == d2DArray.SampleBias(samplerState, float3(0.1f, 0.2f, 0.5f), 0.9f)             // base variant
        && float(0.0f) == d2DArray.SampleBias(samplerState, float3(0.1f, 0.2f, 0.5f), 0.9f, int2(0, 1)) // offset variant
        ;

    // WGSL-COUNT-1: textureSampleBias{{\(+}}dCube_
    result = result
        && float(0.0f) == dCube.SampleBias(samplerState, cubeDir, 0.9f)
        ;

    // WGSL-COUNT-1: textureSampleBias{{\(+}}dCubeArray_
    result = result
        && float(0.0f) == dCubeArray.SampleBias(samplerState, cubeDirAndArrayIndex, 0.9f)
        ;

    return result;
}

bool TEST_DepthTexture_textureSampleCompare()
{
    // WGSL-LABEL: TEST_DepthTexture_textureSampleCompare

    bool result = true;
    float3 cubeDir = normalize(float3(0.3, 0.4, 0.5));
    float4 cubeDirAndArrayIndex = float4(normalize(float3(0.3, 0.4, 0.5)), 1);
    float depthRef = 0.5f;

    // WGSL-COUNT-2: textureSampleCompare{{\(+}}d2D_
    result = result
        && float(0.0f) == d2D.SampleCmp(depthSampler, float2(0.1f, 0.2f), depthRef)             // base variant
        && float(0.0f) == d2D.SampleCmp(depthSampler, float2(0.1f, 0.2f), depthRef, int2(0, 1)) // offset variant
        ;

    // WGSL-COUNT-2: textureSampleCompare{{\(+}}d2DArray_
    result = result
        && float(0.0f) == d2DArray.SampleCmp(depthSampler, float3(0.1f, 0.2f, 0.5f), depthRef)             // base variant
        && float(0.0f) == d2DArray.SampleCmp(depthSampler, float3(0.1f, 0.2f, 0.5f), depthRef, int2(0, 1)) // offset variant
        ;

    // WGSL-COUNT-1: textureSampleCompare{{\(+}}dCube_
    result = result
        && float(0.0f) == dCube.SampleCmp(depthSampler, cubeDir, depthRef)
        ;

    // WGSL-COUNT-1: textureSampleCompare{{\(+}}dCubeArray_
    result = result
        && float(0.0f) == dCubeArray.SampleCmp(depthSampler, cubeDirAndArrayIndex, depthRef)
        ;

    // WGSL-COUNT-2: textureSampleCompareLevel{{\(+}}d2D_
    result = result
        && float(0.0f) == d2D.SampleCmpLevelZero(depthSampler, float2(0.1f, 0.2f), depthRef)             // base variant
        && float(0.0f) == d2D.SampleCmpLevelZero(depthSampler, float2(0.1f, 0.2f), depthRef, int2(0, 1)) // offset variant
        ;

    // WGSL-COUNT-2: textureSampleCompareLevel{{\(+}}d2DArray_
    result = result
        && float(0.0f) == d2DArray.SampleCmpLevelZero(depthSampler, float3(0.1f, 0.2f, 0.5f), depthRef)             // base variant
        && float(0.0f) == d2DArray.SampleCmpLevelZero(depthSampler, float3(0.1f, 0.2f, 0.5f), depthRef, int2(0, 1)) // offset variant
        ;

    // WGSL-COUNT-1: textureSampleCompareLevel{{\(+}}dCube_
    result = result
        && float(0.0f) == dCube.SampleCmpLevelZero(depthSampler, cubeDir, depthRef)
        ;

    // WGSL-COUNT-1: textureSampleCompareLevel{{\(+}}dCubeArray_
    result = result
        && float(0.0f) == dCubeArray.SampleCmpLevelZero(depthSampler, cubeDirAndArrayIndex, depthRef)
        ;

    return result;
}

void fragMain()
{
    // WGSL-LABEL: fragMain

    bool result = true
        && TEST_DepthTexture_getDimensions()
        && TEST_DepthTexture_getMipMapLevels()
        && TEST_DepthTexture_textureSample()
        && TEST_DepthTexture_textureSampleCompare()
        ;

    outputBuffer[0] = int(result);
}
