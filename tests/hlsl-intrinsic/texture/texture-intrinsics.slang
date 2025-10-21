//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=DX11):-slang -compute -dx11 -shaderobj -output-using-type -render-feature hardware-device
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=DX12):-slang -compute -dx12 -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=DX12CS6):-slang -compute -dx12 -profile cs_6_0 -shaderobj -output-using-type -xslang -DCS60
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=VK):-vk -emit-spirv-directly -compute -shaderobj -output-using-type -render-feature hardware-device -xslang -DVK
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type

//TEST_INPUT: Texture1D(size=4, format=D32Float, content = one):name t1D
Texture1D<float> t1D;
//TEST_INPUT: Texture2D(size=4, format=D32Float, content = one):name t2D
Texture2D<float> t2D;
//TEST_INPUT: Texture3D(size=4, format=D32Float, content = one):name t3D
Texture3D<float> t3D;
//TEST_INPUT: TextureCube(size=4, format=D32Float, content = one):name tCube
TextureCube<float> tCube;

//TEST_INPUT: Texture1D(size=4, format=D32Float, content = one, arrayLength=2):name t1DArray
Texture1DArray<float> t1DArray;
//TEST_INPUT: Texture2D(size=4, format=D32Float, content = one, arrayLength=2):name t2DArray
Texture2DArray<float> t2DArray;
//TEST_INPUT: TextureCube(size=4, format=D32Float, content = one, arrayLength=2):name tCubeArray
TextureCubeArray<float> tCubeArray;

//TEST_INPUT: Texture2D(size=4, content = one):name t2D_RGBA
Texture2D<float> t2D_RGBA;
//TEST_INPUT: Texture2D(size=4, content = one, arrayLength=2):name t2DArray_RGBA
Texture2DArray<float> t2DArray_RGBA;
//TEST_INPUT: TextureCube(size=4, content = one):name tCube_RGBA
TextureCube<float> tCube_RGBA;
//TEST_INPUT: TextureCube(size=4, content = one, arrayLength=2):name tCubeArray_RGBA
TextureCubeArray<float> tCubeArray_RGBA;

//TEST_INPUT: Texture1D(size=4, format=D32Float, content = one):name d1D
DepthTexture1D d1D;
//TEST_INPUT: Texture2D(size=4, format=D32Float, content = one):name d2D
DepthTexture2D d2D;
//TEST_INPUT: Texture3D(size=4, format=D32Float, content = one):name d3D
DepthTexture3D d3D;
//TEST_INPUT: TextureCube(size=4, format=D32Float, content = one):name dCube
DepthTextureCube dCube;

//TEST_INPUT: Texture1D(size=4, format=D32Float, content = one, arrayLength=2):name d1DArray
DepthTexture1DArray d1DArray;
//TEST_INPUT: Texture2D(size=4, format=D32Float, content = one, arrayLength=2):name d2DArray
DepthTexture2DArray d2DArray;
//TEST_INPUT: TextureCube(size=4, format=D32Float, content = one, arrayLength=2):name dCubeArray
DepthTextureCubeArray dCubeArray;

//TEST_INPUT: Sampler:name samplerState
SamplerState samplerState;
//TEST_INPUT: Sampler(depthCompare):name shadowSampler
SamplerComparisonState shadowSampler;


//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

// note: the RGBA texture types are not parameterized, since the depth
// texture element is always 'float'
float testOperations<let isShadow : int>(
    int idx,
     _Texture<float, __Shape1D,   0, 0, 0, 0, isShadow, 0, 0> x1D,
     _Texture<float, __Shape2D,   0, 0, 0, 0, isShadow, 0, 0> x2D,
     _Texture<float, __Shape3D,   0, 0, 0, 0, isShadow, 0, 0> x3D,
     _Texture<float, __ShapeCube, 0, 0, 0, 0, isShadow, 0, 0> xCube,
     _Texture<float, __Shape1D,   1, 0, 0, 0, isShadow, 0, 0> x1DArray,
     _Texture<float, __Shape2D,   1, 0, 0, 0, isShadow, 0, 0> x2DArray,
     _Texture<float, __ShapeCube, 1, 0, 0, 0, isShadow, 0, 0> xCubeArray
)
{
    float u = idx * (1.0f / 4);

    float val = 0.0f;

    uint width = 0, height = 0, depth = 0;
    float fwidth = 0.0f, fheight = 0.0f, fdepth = 0.0f;
    uint numLevels = 0, elements = 0;
    float fnumLevels = 0.0f, felements = 0.0f;

    float clamp = 0.0f;
    float bias = 0.0f;

    uint status;

    /*
        <Template Type> Object.SampleLevel()
    */
    val += x1D.SampleLevel(samplerState, u, 0);
    val += x2D.SampleLevel(samplerState, float2(u, u), 0);
    val += x3D.SampleLevel(samplerState, float3(u, u, u), 0);
    val += xCube.SampleLevel(samplerState, normalize(float3(u, 1 - u, u)), 0);

    val += x1DArray.SampleLevel(samplerState, float2(u, 0), 0);
    val += x2DArray.SampleLevel(samplerState, float3(u, u, 0), 0);
    val += xCubeArray.SampleLevel(samplerState, float4(u, u, u, 0), 0);

    // Offset variant
    //  NOTE: The "cpu" profile does not like these, so it's disabled for now
    val += x1D.SampleLevel(samplerState, u, 0, 1);
    val += x2D.SampleLevel(samplerState, float2(u, u), 0, int2(1, 1));
    val += x3D.SampleLevel(samplerState, float3(u, u, u), 0, int3(1, 1, 1));

    val += x1DArray.SampleLevel(samplerState, float2(u, 0), 0, 1);
    val += x2DArray.SampleLevel(samplerState, float3(u, u, 0), 0, int2(1, 1));

    // Status variant
#if !defined(VK)
    val += x1D.SampleLevel(samplerState, u, 0, 1, status);
    val += x2D.SampleLevel(samplerState, float2(u, u), 0, int2(1, 1), status);
    val += x3D.SampleLevel(samplerState, float3(u, u, u), 0, int3(1, 1, 1), status);

    val += x1DArray.SampleLevel(samplerState, float2(u, 0), 0, 1, status);
    val += x2DArray.SampleLevel(samplerState, float3(u, u, 0), 0, int2(1, 1), status);
#endif

    /*
        float Object.SampleCmpLevelZero()
    */
    // NOTE: These are having issues with vulkan (glsl)
    // SampleCmpLevelZero() requires format with VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT.
    val += x1D.SampleCmpLevelZero(shadowSampler, u, 0);
    val += x2D.SampleCmpLevelZero(shadowSampler, float2(u, u), 0);
    val += x1DArray.SampleCmpLevelZero(shadowSampler, float2(u, u), 0);
    val += xCube.SampleCmpLevelZero(shadowSampler, normalize(float3(u, 1 - u, u)), 0);
    val += x2DArray.SampleCmpLevelZero(shadowSampler, normalize(float3(u, 1 - u, u)), 0);
    val += xCubeArray.SampleCmpLevelZero(shadowSampler, normalize(float4(u, 1-u, u, u)), 0);

    // Offset variant
    val += x1D.SampleCmpLevelZero(shadowSampler, u, 0, 0);
    val += x2D.SampleCmpLevelZero(shadowSampler, float2(u, u), 0, int2(0, 0));
    // TextureCube does not have an offset version of this

    // Status variant
    #if !defined(VK)

    val += x1D.SampleCmpLevelZero(shadowSampler, u, 0, 0, status);
    val += x2D.SampleCmpLevelZero(shadowSampler, float2(u, u), 0, int2(0, 0), status);

    val += x1DArray.SampleCmpLevelZero(shadowSampler, float2(u, u), 0, 0, status);
    val += x2DArray.SampleCmpLevelZero(shadowSampler, normalize(float3(u, 1 - u, u)), 0, int2(0, 0), status);
#endif

    /*
        float Object.SampleCmpLevel().
        These require SM 6.7 for dx12 but functional compute tests currently do not run with the cs_6_7,
        hence only enable these for vk.
    */
#if defined(VK)
    float level = 1.0;
    val += x1D.SampleCmpLevel(shadowSampler, u, 0, level);
    val += x2D.SampleCmpLevel(shadowSampler, float2(u, u), 0, level);
    val += x1DArray.SampleCmpLevel(shadowSampler, float2(u, u), 0, level);
    val += xCube.SampleCmpLevel(shadowSampler, normalize(float3(u, 1 - u, u)), 0, level);
    val += x2DArray.SampleCmpLevel(shadowSampler, normalize(float3(u, 1 - u, u)), 0, level);
    val += xCubeArray.SampleCmpLevel(shadowSampler, normalize(float4(u, 1-u, u, u)), 0, level);

    // Offset variant
    val += x1D.SampleCmpLevel(shadowSampler, u, 0, level, 0);
    val += x2D.SampleCmpLevel(shadowSampler, float2(u, u), 0, level, int2(0, 0));
#endif

    /*
        void Object.GetDimensions()
    */
    x1D.GetDimensions(width);
    val += width;

    x1D.GetDimensions(fwidth);
    val += fwidth;

    x1D.GetDimensions(0, width, numLevels);
    val += width;
    val += numLevels;

    x1D.GetDimensions(0, fwidth, fnumLevels);
    val += fwidth;
    val += fnumLevels;

    x2D.GetDimensions(width, height);
    val += width;
    val += height;

    x2D.GetDimensions(fwidth, fheight);
    val += fwidth;
    val += fheight;

    x2D.GetDimensions(0, width, height, numLevels);
    val += width;
    val += height;
    val += numLevels;

    x2D.GetDimensions(0, fwidth, fheight, fnumLevels);
    val += fwidth;
    val += fheight;
    val += fnumLevels;

    x3D.GetDimensions(width, height, depth);
    val += width;
    val += height;
    val += depth;

    x3D.GetDimensions(fwidth, fheight, fdepth);
    val += fwidth;
    val += fheight;
    val += fdepth;

    x3D.GetDimensions(0, width, height, depth, numLevels);
    val += width;
    val += height;
    val += depth;
    val += numLevels;

    x3D.GetDimensions(0, fwidth, fheight, fdepth, fnumLevels);
    val += fwidth;
    val += fheight;
    val += fdepth;
    val += fnumLevels;

    xCube.GetDimensions(width, height);
    val += width;
    val += height;

    xCube.GetDimensions(fwidth, fheight);
    val += fwidth;
    val += fheight;

    xCube.GetDimensions(0, width, height, numLevels);
    val += width;
    val += height;
    val += numLevels;

    xCube.GetDimensions(0, fwidth, fheight, fnumLevels);
    val += fwidth;
    val += fheight;
    val += fnumLevels;

    x1DArray.GetDimensions(width, elements);
    val += width;
    val += elements;

    x1DArray.GetDimensions(fwidth, felements);
    val += fwidth;
    val += felements;

    x1DArray.GetDimensions(0, width, elements, numLevels);
    val += width;
    val += elements;
    val += numLevels;

    x1DArray.GetDimensions(0, fwidth, felements, fnumLevels);
    val += fwidth;
    val += felements;
    val += fnumLevels;

    x2DArray.GetDimensions(width, height, elements);
    val += width;
    val += height;
    val += elements;

    x2DArray.GetDimensions(fwidth, fheight, felements);
    val += fwidth;
    val += fheight;
    val += felements;

    x2DArray.GetDimensions(0, width, height, elements, numLevels);
    val += width;
    val += height;
    val += elements;
    val += numLevels;

    x2DArray.GetDimensions(0, fwidth, fheight, felements, fnumLevels);
    val += fwidth;
    val += fheight;
    val += felements;
    val += fnumLevels;

    xCubeArray.GetDimensions(width, height, elements);
    val += width;
    val += height;
    val += elements;

    // fxc 47.0: (95): error X4598:  The array element count of GetDimensions on TextureCubeArray objects is unavailable on cs_5_0
#if defined(CS60) || defined(VK)
    xCubeArray.GetDimensions(fwidth, fheight, felements);
    val += fwidth;
    val += fheight;
    val += felements;
#endif

    xCubeArray.GetDimensions(0, width, height, elements, numLevels);
    val += width;
    val += height;
    val += elements;
    val += numLevels;

#if defined(CS60) || defined(VK)
    xCubeArray.GetDimensions(0, fwidth, fheight, felements, fnumLevels);
    val += fwidth;
    val += fheight;
    val += felements;
    val += fnumLevels;
#endif

    /*
        <Template Type>4 Object.Gather()
    */
    float4 f4 = t2D_RGBA.Gather(samplerState, float2(u, u));
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    f4 = tCube_RGBA.Gather(samplerState, normalize(float3(u, 1 - u, u)));
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    f4 = t2DArray_RGBA.Gather(samplerState, float3(u, u, 0));
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    f4 = tCubeArray_RGBA.Gather(samplerState, float4(normalize(float3(u, 1 - u, u)), 0));
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    // Offset variant
    f4 = t2D_RGBA.Gather(samplerState, float2(u, u), int2(0, 0));
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    f4 = t2DArray_RGBA.Gather(samplerState, float3(u, u, 0), int2(0, 0));
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    // Object.GatherGreen()
    f4 = t2D_RGBA.GatherGreen(samplerState, float2(u, u), int2(1, 2));
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    f4 = t2D_RGBA.GatherGreen(samplerState, float2(u, u), int2(1, 2), int2(3, 4), int2(5, 6), int2(7, 8));
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    f4 = tCubeArray_RGBA.GatherGreen(samplerState, float4(1.5, 1.5, 1.5, 1.5));
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    // status variant
#if !defined(VK)
    f4 = t2D_RGBA.GatherGreen(samplerState, float2(u, u), int2(1, 2), status);
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    f4 = t2D_RGBA.GatherGreen(samplerState, float2(u, u), int2(1, 2), int2(3, 4), int2(5, 6), int2(7, 8), status);
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;

    f4 = tCubeArray_RGBA.GatherGreen(samplerState, float4(1.5, 1.5, 1.5, 1.5), status);
    val += f4.x;  val += f4.y;  val += f4.z;  val += f4.w;
#endif

    /*
        ret Object.Load()
    */
    val += x1D.Load(int2(0, 0));
    val += x2D.Load(int3(0, 0, 0));
    val += x3D.Load(int4(0, 0, 0, 0));

    val += x1DArray.Load(int3(0, 0, 0));
    val += x2DArray.Load(int4(0, 0, 0, 0));

    // Offset variant
    val += x1D.Load(int2(0, 0), 0);
    val += x2D.Load(int3(0, 0, 0), int2(0,0));
    val += x3D.Load(int4(0, 0, 0, 0), int3(0, 0, 0));

    val += x1DArray.Load(int3(0, 0, 0), 0);
    val += x2DArray.Load(int4(0, 0, 0, 0), int2(0, 0));

    /*
        <Template Type> Object.SampleGrad()
    */
    float _ddx = 0.0f, _ddy = 0.0f;

    val += x1D.SampleGrad(samplerState, 0.0f, _ddx, _ddy);
    val += x2D.SampleGrad(samplerState, float2(u, u), float2(_ddx, _ddx), float2(_ddy, _ddy));
    val += x3D.SampleGrad(samplerState, float3(u, u, u), float3(_ddx, _ddx, _ddx), float3(_ddy, _ddy, _ddy));
    val += xCube.SampleGrad(samplerState, normalize(float3(u, 1 - u, u)), float3(_ddx, _ddx, _ddx), float3(_ddy, _ddy, _ddy));

    val += x1DArray.SampleGrad(samplerState, float2(0.0f, 0.0f), _ddx, _ddy);
    val += x2DArray.SampleGrad(samplerState, float3(u, u, 0.0f), float2(_ddx, _ddx), float2(_ddy, _ddy));

    // Offset variant
    val += x1D.SampleGrad(samplerState, 0.0f, _ddx, _ddy, 0);
    val += x2D.SampleGrad(samplerState, float2(u, u), float2(_ddx, _ddx), float2(_ddy, _ddy), int2(0, 0));
    val += x3D.SampleGrad(samplerState, float3(u, u, u), float3(_ddx, _ddx, _ddx), float3(_ddy, _ddy, _ddy), int3(0, 0, 0));

    val += x1DArray.SampleGrad(samplerState, float2(0.0f, 0.0f), _ddx, _ddy, 0);
    val += x2DArray.SampleGrad(samplerState, float3(u, u, 0.0f), float2(_ddx, _ddx), float2(_ddy, _ddy), int2(0, 0));

    // Status variant
#if !defined(VK)
    val += x1D.SampleGrad(samplerState, 0.0f, _ddx, _ddy, 0, clamp,  status);
    val += x2D.SampleGrad(samplerState, float2(u, u), float2(_ddx, _ddx), float2(_ddy, _ddy), int2(0, 0), clamp, status);
    val += x3D.SampleGrad(samplerState, float3(u, u, u), float3(_ddx, _ddx, _ddx), float3(_ddy, _ddy, _ddy), int3(0, 0, 0), clamp, status);

    val += x1DArray.SampleGrad(samplerState, float2(0.0f, 0.0f), _ddx, _ddy, 0, clamp, status);
    val += x2DArray.SampleGrad(samplerState, float3(u, u, 0.0f), float2(_ddx, _ddx), float2(_ddy, _ddy), int2(0, 0), clamp, status);
#endif

    return val;
}

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    float val = 0.0f;

    val += testOperations<0>(idx, t1D, t2D, t3D, tCube, t1DArray, t2DArray, tCubeArray);
    val += testOperations<1>(idx, d1D, d2D, d3D, dCube, d1DArray, d2DArray, dCubeArray);

    outputBuffer[idx] = val;
}

// DX11: 708
// DX11: 708
// DX11: 708
// DX11: 708
// DX12: 708
// DX12: 708
// DX12: 708
// DX12: 708
// DX12CS6: 754
// DX12CS6: 754
// DX12CS6: 754
// DX12CS6: 754
// VK: 718
// VK: 718
// VK: 718
// VK: 718
