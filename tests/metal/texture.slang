//TEST:SIMPLE(filecheck=METAL_CAPS_ERROR): -stage compute -entry computeMain -target metal -DEMIT_SOURCE -DNEED_TO_TEST_FOR_METAL_CAPABILITY
//TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal -DEMIT_SOURCE
//TEST:SIMPLE(filecheck=METALLIB): -stage compute -entry computeMain -target metallib -DEMIT_SOURCE -DMETALLIB
//TEST:SIMPLE(filecheck=HLSL): -stage compute -entry computeMain -target hlsl -DEMIT_SOURCE
//TEST:SIMPLE(filecheck=SPIR): -stage compute -entry computeMain -target spirv -emit-spirv-directly -DEMIT_SOURCE

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=FUNCTIONAL):-slang -compute -dx12 -profile cs_6_6 -use-dxil -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=FUNCTIONAL):-vk -emit-spirv-directly -compute -shaderobj -output-using-type -render-feature hardware-device

// METAL_CAPS_ERROR: error 36107

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture1D(size=4, content = one):name t1D
Texture1D<float> t1D;
//TEST_INPUT: Texture2D(size=4, content = one):name t2D
Texture2D<float> t2D;
//TEST_INPUT: Texture3D(size=4, content = one):name t3D
Texture3D<float> t3D;
//TEST_INPUT: TextureCube(size=4, content = one):name tCube
TextureCube<float> tCube;

//TEST_INPUT: Texture1D(size=4, content = one, arrayLength=2):name t1DArray
Texture1DArray<float> t1DArray;
//TEST_INPUT: Texture2D(size=4, content = one, arrayLength=2):name t2DArray
Texture2DArray<float> t2DArray;
//TEST_INPUT: TextureCube(size=4, content = one, arrayLength=2):name tCubeArray
TextureCubeArray<float> tCubeArray;

// Metal doc says "For depth texture types, T must be float."
__generic<T : __BuiltinType, let sampleCount:int=0, let format:int=0>
typealias depth2d = __TextureImpl<
    T,
    __Shape2D,
    0, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    0, // isCombined
    format
>;

__generic<T : __BuiltinType, let sampleCount:int=0, let format:int=0>
typealias depth2d_array = __TextureImpl<
    T,
    __Shape2D,
    1, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    0, // isCombined
    format
>;

__generic<T : __BuiltinType, let sampleCount:int=0, let format:int=0>
typealias depthcube = __TextureImpl<
    T,
    __ShapeCube,
    0, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    0, // isCombined
    format
>;

__generic<T : __BuiltinType, let sampleCount:int=0, let format:int=0>
typealias depthcube_array = __TextureImpl<
    T,
    __ShapeCube,
    1, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    0, // isCombined
    format
>;

//TEST_INPUT: Texture2D(size=4, content = one):name d2D
depth2d<float> d2D;
//TEST_INPUT: TextureCube(size=4, content = one):name dCube
depthcube<float> dCube;
//TEST_INPUT: Texture2D(size=4, content = one, arrayLength=2):name d2DArray
depth2d_array<float> d2DArray;
//TEST_INPUT: TextureCube(size=4, content = one, arrayLength=2):name dCubeArray
depthcube_array<float> dCubeArray;

//TEST_INPUT: Sampler:name samplerState
SamplerState samplerState;
//TEST_INPUT: Sampler:name shadowSampler
SamplerComparisonState shadowSampler;


bool TEST_texture_float()
{
    // Metal textures support `Tv` types, which "denotes a 4-component vector
    // type based on the templated type <T> for declaring the texture type:
    //  - If T is float, Tv is float4.
    //  - If T is half, Tv is half4.
    //  - If T is int, Tv is int4.
    //  - If T is uint, Tv is uint4.
    //  - If T is short, Tv is short4.
    //  - If T is ushort, Tv is ushort4."
    typealias Tv = vector<float,4>;

    float u = 0;
    float u2 = 0.5;
    constexpr const float ddx = 0.0f;
    constexpr const float ddy = 0.0f;

    uint width = 0, height = 0, depth = 0;
    float fwidth = 0.0f, fheight = 0.0f, fdepth = 0.0f;
    uint numLevels = 0, elements = 0, sampleCount = 0;
    float fnumLevels = 0.0f, felements = 0.0f;

    bool voidResult = true;

    // ======================
    //  void GetDimensions()
    // ======================

    // METAL: .get_width(
    // METALLIB: call {{.*}}.get_width_texture_1d(
    t1D.GetDimensions(width);
    voidResult = voidResult && (uint(4) == width);

    // METAL: .get_width({{.*}}.get_num_mip_levels()
    // METALLIB: call {{.*}}.get_num_mip_levels_texture_1d(
    t1D.GetDimensions(0, width, numLevels);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(3) == numLevels);

    // METAL: .get_width({{.*}}.get_height(
    // METALLIB: call {{.*}}.get_height_texture_2d(
    t2D.GetDimensions(width, height);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);

    // METAL: .get_width({{.*}}.get_height({{.*}}.get_num_mip_levels()
    // METALLIB: call {{.*}}.get_num_mip_levels_texture_2d(
    t2D.GetDimensions(0, width, height, numLevels);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(3) == numLevels);

    // METAL: .get_width({{.*}}.get_height({{.*}}.get_depth(
    // METALLIB: call {{.*}}.get_depth_texture_3d(
    t3D.GetDimensions(width, height, depth);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(4) == depth);

    // METAL: .get_width({{.*}}.get_height({{.*}}.get_depth({{.*}}.get_num_mip_levels()
    // METALLIB: call {{.*}}.get_num_mip_levels_texture_3d(
    t3D.GetDimensions(0, width, height, depth, numLevels);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(4) == depth);
    voidResult = voidResult && (uint(3) == numLevels);

    // METAL: .get_width({{.*}}.get_height({{.*}}
    // METALLIB: call {{.*}}.get_height_texture_cube(
    tCube.GetDimensions(width, height);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);

    // METAL: .get_width({{.*}}.get_height({{.*}}.get_num_mip_levels()
    // METALLIB: call {{.*}}.get_num_mip_levels_texture_cube(
    tCube.GetDimensions(0, width, height, numLevels);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(3) == numLevels);

    // METAL: .get_width({{.*}}.get_array_size(
    // METALLIB: call {{.*}}.get_array_size_texture_1d_array(
    t1DArray.GetDimensions(width, elements);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(2) == elements);

    // METAL: .get_width({{.*}}.get_num_mip_levels(
    // METALLIB: call {{.*}}.get_num_mip_levels_texture_1d_array(
    t1DArray.GetDimensions(0, width, elements, numLevels);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(2) == elements);
    voidResult = voidResult && (uint(3) == numLevels);

    // METAL: .get_width({{.*}}.get_height({{.*}}.get_array_size(
    // METALLIB: call {{.*}}.get_array_size_texture_2d_array(
    t2DArray.GetDimensions(width, height, elements);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(2) == elements);

    // METAL: .get_width({{.*}}.get_height({{.*}}.get_num_mip_levels(
    // METALLIB: call {{.*}}.get_num_mip_levels_texture_2d_array(
    t2DArray.GetDimensions(0, width, height, elements, numLevels);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(2) == elements);
    voidResult = voidResult && (uint(3) == numLevels);

    // METAL: .get_width({{.*}}.get_height({{.*}}.get_array_size(
    // METALLIB: call {{.*}}.get_array_size_texture_cube_array(
    tCubeArray.GetDimensions(width, height, elements);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(2) == elements);

    // METAL: .get_width({{.*}}.get_height({{.*}}.get_num_mip_levels(
    // METALLIB: call {{.*}}.get_num_mip_levels_texture_cube_array(
    tCubeArray.GetDimensions(0, width, height, elements, numLevels);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(2) == elements);
    voidResult = voidResult && (uint(3) == numLevels);

    bool result = voidResult
        // ===============================
        // float CalculateLevelOfDetail()
        // ===============================

#if defined(NEED_TO_TEST_FOR_METAL_CAPABILITY)
        // Metal doesn't support LOD for 1D texture
        && float(0) == t1D.CalculateLevelOfDetail(samplerState, u)
#endif

        // METAL: t2D{{.*}}.calculate_clamped_lod({{.*}}
        // METALLIB: call {{.*}}.calculate_clamped_lod_texture_2d(
        && float(0) == t2D.CalculateLevelOfDetail(samplerState, float2(u, u))

        // METAL: t3D{{.*}}.calculate_clamped_lod({{.*}}
        // METALLIB: call {{.*}}.calculate_clamped_lod_texture_3d(
        && float(0) == t3D.CalculateLevelOfDetail(samplerState, float3(u, u, u))

        // METAL: tCube{{.*}}.calculate_clamped_lod({{.*}}
        // METALLIB: call {{.*}}.calculate_clamped_lod_texture_cube(
        && float(0) == tCube.CalculateLevelOfDetail(samplerState, normalize(float3(u, 1 - u, u)))

        // METAL: t2DArray{{.*}}.calculate_clamped_lod({{.*}}
        // METALLIB: call {{.*}}.calculate_clamped_lod_texture_2d_array(
        && float(0) == t2DArray.CalculateLevelOfDetail(samplerState, float2(u, u))

        // METAL: tCubeArray{{.*}}.calculate_clamped_lod({{.*}}
        // METALLIB: call {{.*}}.calculate_clamped_lod_texture_cube_array(
        && float(0) == tCubeArray.CalculateLevelOfDetail(samplerState, normalize(float3(u, 1 - u, u)))

        // ========================================
        // float CalculateLevelOfDetailUnclamped()
        // ========================================

#if defined(NEED_TO_TEST_FOR_METAL_CAPABILITY)
        // Metal doesn't support LOD for 1D texture
        && float(0) >= t1D.CalculateLevelOfDetailUnclamped(samplerState, u)
#endif

        // METAL: t2D{{.*}}.calculate_unclamped_lod({{.*}}
        // METALLIB: call {{.*}}.calculate_unclamped_lod_texture_2d(
        && float(0) >= t2D.CalculateLevelOfDetailUnclamped(samplerState, float2(u, u))

        // METAL: t3D{{.*}}.calculate_unclamped_lod({{.*}}
        // METALLIB: call {{.*}}.calculate_unclamped_lod_texture_3d(
        && float(0) >= t3D.CalculateLevelOfDetailUnclamped(samplerState, float3(u, u, u))

        // METAL: tCube{{.*}}.calculate_unclamped_lod({{.*}}
        // METALLIB: call {{.*}}.calculate_unclamped_lod_texture_cube(
        && float(0) >= tCube.CalculateLevelOfDetailUnclamped(samplerState, normalize(float3(u, 1 - u, u)))

        // METAL: t2DArray{{.*}}.calculate_unclamped_lod({{.*}}
        // METALLIB: call {{.*}}.calculate_unclamped_lod_texture_2d_array(
        && float(0) >= t2DArray.CalculateLevelOfDetailUnclamped(samplerState, float2(u, u))

        // METAL: tCubeArray{{.*}}.calculate_unclamped_lod({{.*}}
        // METALLIB: call {{.*}}.calculate_unclamped_lod_texture_cube_array(
        && float(0) >= tCubeArray.CalculateLevelOfDetailUnclamped(samplerState, normalize(float3(u, 1 - u, u)))

        // ===========
        // T Sample()
        // ===========

        // METAL: t1D{{.*}}.sample(
        // METALLIB: call {{.*}}.sample_texture_1d.v4f32(
        && all(Tv(1) == t1D.Sample(samplerState, u))

        // METAL: t2D{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_2d.v4f32(
        && all(Tv(1) == t2D.Sample(samplerState, float2(u, u)))

        // METAL: t3D{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_3d.v4f32(
        && all(Tv(1) == t3D.Sample(samplerState, float3(u, u, u)))

        // METAL: tCube{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_cube.v4f32(
        && all(Tv(1) == tCube.Sample(samplerState, normalize(float3(u, 1 - u, u))))

        // METAL: t1DArray{{.*}}.sample(
        // METALLIB: call {{.*}}.sample_texture_1d_array.v4f32(
        && all(Tv(1) == t1DArray.Sample(samplerState, float2(u, 0)))

        // METAL: t2DArray{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_2d_array.v4f32(
        && all(Tv(1) == t2DArray.Sample(samplerState, float3(u, u, 0)))

        // METAL: tCubeArray{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_cube_array.v4f32(
        && all(Tv(1) == tCubeArray.Sample(samplerState, float4(normalize(float3(u, 1 - u, u)), 0)))

        // Offset variant

#if defined(NEED_TO_TEST_FOR_METAL_CAPABILITY)
        // Metal doesn't support Offset for 1D and Cube texture
        && all(Tv(1) == t1D.Sample(samplerState, u2, 1))
        && all(Tv(1) == t1DArray.Sample(samplerState, float2(u2, 0), 1))
#endif

        // METAL: t2D{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_2d.v4f32(
        && all(Tv(1) == t2D.Sample(samplerState, float2(u, u), int2(1, 1)))

        // METAL: t3D{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_3d.v4f32(
        && all(Tv(1) == t3D.Sample(samplerState, float3(u, u, u), int3(1, 1, 1)))

        // METAL: t2DArray{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_2d_array.v4f32(
        && all(Tv(1) == t2DArray.Sample(samplerState, float3(u, u, 0), int2(1, 1)))

        // Clamp variant

#if defined(NEED_TO_TEST_FOR_METAL_CAPABILITY)
        // Metal doesn't support Offset for 1D and Cube texture
        && all(Tv(1) == t1D.Sample(samplerState, u2, 1, float(1)))
        && all(Tv(1) == t1DArray.Sample(samplerState, float2(u2, 0), 1, float(1)))
#endif

        // METAL: t2D{{.*}}.sample({{.*}} min_lod_clamp(
        // METALLIB: call {{.*}}.sample_texture_2d.v4f32(
        && all(Tv(1) == t2D.Sample(samplerState, float2(u, u), int2(1, 1), float(1)))

        // METAL: t3D{{.*}}.sample({{.*}} min_lod_clamp(
        // METALLIB: call {{.*}}.sample_texture_3d.v4f32(
        && all(Tv(1) == t3D.Sample(samplerState, float3(u, u, u), int3(1, 1, 1), float(1)))

        // METAL: t2DArray{{.*}}.sample({{.*}} min_lod_clamp(
        // METALLIB: call {{.*}}.sample_texture_2d_array.v4f32(
        && all(Tv(1) == t2DArray.Sample(samplerState, float3(u, u, 0), int2(1, 1), float(1)))

        // ===============
        // T SampleBias()
        // ===============

#if defined(NEED_TO_TEST_FOR_METAL_CAPABILITY)
        // Metal doesn't support Bias for 1D texture
        && all(Tv(1) == t1D.SampleBias(samplerState, u, float(-1)))
        && all(Tv(1) == t1DArray.SampleBias(samplerState, float2(u, 0), float(-1)))
#endif

        // METAL: t2D{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_2d.v4f32(
        && all(Tv(1) == t2D.SampleBias(samplerState, float2(u, u), float(-1)))

        // METAL: t3D{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_3d.v4f32(
        && all(Tv(1) == t3D.SampleBias(samplerState, float3(u, u, u), float(-1)))

        // METAL: tCube{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_cube.v4f32(
        && all(Tv(1) == tCube.SampleBias(samplerState, normalize(float3(u, 1 - u, u)), float(-1)))

        // METAL: t2DArray{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_2d_array.v4f32(
        && all(Tv(1) == t2DArray.SampleBias(samplerState, float3(u, u, 0), float(-1)))

        // METAL: tCubeArray{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_cube_array.v4f32(
        && all(Tv(1) == tCubeArray.SampleBias(samplerState, float4(normalize(float3(u, 1 - u, u)), 0), float(-1)))

        // Offset variant

#if defined(NEED_TO_TEST_FOR_METAL_CAPABILITY)
        // Metal doesn't support Offset for 1D and Cube texture
        && all(Tv(1) == t1D.SampleBias(samplerState, u2, float(-1), 1))
        && all(Tv(1) == t1DArray.SampleBias(samplerState, float2(u2, 0), float(-1), 1))
#endif

        // METAL: t2D{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_2d.v4f32(
        && all(Tv(1) == t2D.SampleBias(samplerState, float2(u, u), float(-1), int2(1, 1)))

        // METAL: t3D{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_3d.v4f32(
        && all(Tv(1) == t3D.SampleBias(samplerState, float3(u, u, u), float(-1), int3(1, 1, 1)))

        // METAL: t2DArray{{.*}}.sample({{.*}}
        // METALLIB: call {{.*}}.sample_texture_2d_array.v4f32(
        && all(Tv(1) == t2DArray.SampleBias(samplerState, float3(u, u, 0), float(-1), int2(1, 1)))

        // ===================================
        //  T SampleLevel()
        // ===================================

#if defined(NEED_TO_TEST_FOR_METAL_CAPABILITY)
        // Metal doesn't support LOD for 1D texture
        && all(Tv(1) == t1D.SampleLevel(samplerState, u, 0))
        && all(Tv(1) == t1DArray.SampleLevel(samplerState, float2(u, 0), 0))
#endif

        // METAL: t2D{{.*}}.sample({{.*}} level(
        // METALLIB: call {{.*}}.sample_texture_2d.v4f32(
        && all(Tv(1) == t2D.SampleLevel(samplerState, float2(u, u), 0))

        // METAL: t3D{{.*}}.sample({{.*}} level(
        // METALLIB: call {{.*}}.sample_texture_3d.v4f32(
        && all(Tv(1) == t3D.SampleLevel(samplerState, float3(u, u, u), 0))

        // METAL: tCube{{.*}}.sample({{.*}} level(
        // METALLIB: call {{.*}}.sample_texture_cube.v4f32(
        && all(Tv(1) == tCube.SampleLevel(samplerState, normalize(float3(u, 1 - u, u)), 0))

        // METAL: t2DArray{{.*}}.sample({{.*}} level(
        // METALLIB: call {{.*}}.sample_texture_2d_array.v4f32(
        && all(Tv(1) == t2DArray.SampleLevel(samplerState, float3(u, u, 0), 0))

        // METAL: tCubeArray{{.*}}.sample({{.*}} level(
        // METALLIB: call {{.*}}.sample_texture_cube_array.v4f32(
        && all(Tv(1) == tCubeArray.SampleLevel(samplerState, float4(normalize(float3(u, 1 - u, u)), 0), 0))

        // Offset variant

#if defined(NEED_TO_TEST_FOR_METAL_CAPABILITY)
        // Metal doesn't support LOD for 1D texture
        && all(Tv(1) == t1D.SampleLevel(samplerState, u2, 0, 1))
        && all(Tv(1) == t1DArray.SampleLevel(samplerState, float2(u2, 0), 0, 1))
#endif

        // METAL: t2D{{.*}}.sample({{.*}} level(
        // METALLIB: call {{.*}}.sample_texture_2d.v4f32(
        && all(Tv(1) == t2D.SampleLevel(samplerState, float2(u, u), 0, int2(1, 1)))

        // METAL: t3D{{.*}}.sample({{.*}} level(
        // METALLIB: call {{.*}}.sample_texture_3d.v4f32(
        && all(Tv(1) == t3D.SampleLevel(samplerState, float3(u, u, u), 0, int3(1, 1, 1)))

        // METAL: t2DArray{{.*}}.sample({{.*}} level(
        // METALLIB: call {{.*}}.sample_texture_2d_array.v4f32(
        && all(Tv(1) == t2DArray.SampleLevel(samplerState, float3(u, u, 0), 0, int2(1, 1)))

        // ==================
        // float SampleCmp()
        // ==================

        // METAL: d2D{{.*}}.sample_compare(
        // METALLIB: call {{.*}}.sample_compare_depth_2d.f32(
        && float(1) == d2D.SampleCmp(shadowSampler, float2(u, u), 0)

        // METAL: d2DArray{{.*}}.sample_compare(
        // METALLIB: call {{.*}}.sample_compare_depth_2d_array.f32(
        && float(1) == d2DArray.SampleCmp(shadowSampler, float3(u, u, 0), 0)

        // METAL: dCube{{.*}}.sample_compare(
        // METALLIB: call {{.*}}.sample_compare_depth_cube.f32(
        && float(1) == dCube.SampleCmp(shadowSampler, normalize(float3(u, 1 - u, u)), 0)

        // METAL: dCubeArray{{.*}}.sample_compare(
        // METALLIB: call {{.*}}.sample_compare_depth_cube_array.f32(
        && float(1) == dCubeArray.SampleCmp(shadowSampler, float4(normalize(float3(u, 1 - u, u)), 0), 0)

        // Offset variant

        // METAL: d2D{{.*}}.sample_compare(
        // METALLIB: call {{.*}}.sample_compare_depth_2d.f32(
        && float(1) == d2D.SampleCmp(shadowSampler, float2(u2, u), 0, int2(0, 0))

        // ===================================
        //  float SampleCmpLevelZero()
        // ===================================

        // METAL: d2D{{.*}}.sample_compare(
        // METALLIB: call {{.*}}.sample_compare_depth_2d.f32(
        && float(1) == d2D.SampleCmpLevelZero(shadowSampler, float2(u, u), 0)

        // METAL: d2DArray{{.*}}.sample_compare(
        // METALLIB: call {{.*}}.sample_compare_depth_2d_array.f32(
        && float(1) == d2DArray.SampleCmpLevelZero(shadowSampler, float3(u, u, 0), 0)

        // METAL: dCube{{.*}}.sample_compare(
        // METALLIB: call {{.*}}.sample_compare_depth_cube.f32(
        && float(1) == dCube.SampleCmpLevelZero(shadowSampler, normalize(float3(u, 1 - u, u)), 0)

        // METAL: dCubeArray{{.*}}.sample_compare(
        // METALLIB: call {{.*}}.sample_compare_depth_cube_array.f32(
        && float(1) == dCubeArray.SampleCmpLevelZero(shadowSampler, float4(normalize(float3(u, 1-u, u)), 0), 0)

        // Offset variant

        // METAL: d2D{{.*}}.sample_compare(
        // METALLIB: call {{.*}}.sample_compare_depth_2d.f32(
        && float(1) == d2D.SampleCmpLevelZero(shadowSampler, float2(u2, u), 0, int2(0, 0))

        // ==================================
        //  vector<T,4> Gather()
        // ==================================

        // METAL: t2D{{.*}}.gather(
        // METALLIB: call {{.*}}.gather_texture_2d.v4f32(
        && all(Tv(1) == t2D.Gather(samplerState, float2(u, u)))

        // METAL: tCube{{.*}}.gather(
        // METALLIB: call {{.*}}.gather_texture_cube.v4f32(
        && all(Tv(1) == tCube.Gather(samplerState, normalize(float3(u, 1 - u, u))))

        // METAL: t2DArray{{.*}}.gather(
        // METALLIB: call {{.*}}.gather_texture_2d_array.v4f32(
        && all(Tv(1) == t2DArray.Gather(samplerState, float3(u, u, 0)))

        // METAL: tCubeArray{{.*}}.gather(
        // METALLIB: call {{.*}}.gather_texture_cube_array.v4f32(
        && all(Tv(1) == tCubeArray.Gather(samplerState, float4(normalize(float3(u, 1 - u, u)), 0)))

        // Offset variant

        // METAL: t2D{{.*}}.gather(
        // METALLIB: call {{.*}}.gather_texture_2d.v4f32(
        && all(Tv(1) == t2D.Gather(samplerState, float2(u2, u), int2(0, 0)))

        // METAL: t2DArray{{.*}}.gather(
        // METALLIB: call {{.*}}.gather_texture_2d_array.v4f32(
        && all(Tv(1) == t2DArray.Gather(samplerState, float3(u2, u, 0), int2(0, 0)))

        // =====================================
        //  T SampleGrad()
        // =====================================

#if defined(NEED_TO_TEST_FOR_METAL_CAPABILITY)
        // Metal doesn't support LOD for 1D texture
        && all(Tv(1) == t1D.SampleGrad(samplerState, 0.0f, ddx, ddy))
        && all(Tv(1) == t1DArray.SampleGrad(samplerState, float2(0.0f, 0.0f), ddx, ddy))
        && all(Tv(1) == t1D.SampleGrad(samplerState, 0.0f, ddx, ddy, 0))
        && all(Tv(1) == t1DArray.SampleGrad(samplerState, float2(0.0f, 0.0f), ddx, ddy, 0))
#endif

        // METAL: t2D{{.*}}.sample(
        // METALLIB: call {{.*}}.sample_texture_2d_grad.v4f32(
        && all(Tv(1) == t2D.SampleGrad(samplerState, float2(u, u), float2(ddx, ddx), float2(ddy, ddy)))

        // METAL: t3D{{.*}}.sample(
        // METALLIB: call {{.*}}.sample_texture_3d_grad.v4f32(
        && all(Tv(1) == t3D.SampleGrad(samplerState, float3(u, u, u), float3(ddx, ddx, ddx), float3(ddy, ddy, ddy)))

        // METAL: tCube{{.*}}.sample(
        // METALLIB: call {{.*}}.sample_texture_cube_grad.v4f32(
        && all(Tv(1) == tCube.SampleGrad(samplerState, normalize(float3(u, 1 - u, u)), float3(ddx, ddx, ddx), float3(ddy, ddy, ddy)))

        // METAL: t2DArray{{.*}}.sample(
        // METALLIB: call {{.*}}.sample_texture_2d_array_grad.v4f32(
        && all(Tv(1) == t2DArray.SampleGrad(samplerState, float3(u, u, 0.0f), float2(ddx, ddx), float2(ddy, ddy)))

        // Offset variant

        // METAL: t2D{{.*}}.sample(
        // METALLIB: call {{.*}}.sample_texture_2d_grad.v4f32(
        && all(Tv(1) == t2D.SampleGrad(samplerState, float2(u2, u), float2(ddx, ddx), float2(ddy, ddy), int2(0, 0)))

        // METAL: t3D{{.*}}.sample(
        // METALLIB: call {{.*}}.sample_texture_3d_grad.v4f32(
        && all(Tv(1) == t3D.SampleGrad(samplerState, float3(u2, u, u), float3(ddx, ddx, ddx), float3(ddy, ddy, ddy), int3(0, 0, 0)))

        // METAL: t2DArray{{.*}}.sample(
        // METALLIB: call {{.*}}.sample_texture_2d_array_grad.v4f32(
        && all(Tv(1) == t2DArray.SampleGrad(samplerState, float3(u2, u, 0.0f), float2(ddx, ddx), float2(ddy, ddy), int2(0, 0)))

        // ===================
        //  T Load()
        // ===================

        // METAL: t1D{{.*}}.read(
        // METALLIB: call {{.*}}.read_texture_1d.v4f32(
        && all(Tv(1) == t1D.Load(int2(0, 0)))

        // METAL: t2D{{.*}}.read(
        // METALLIB: call {{.*}}.read_texture_2d.v4f32(
        && all(Tv(1) == t2D.Load(int3(0, 0, 0)))

        // METAL: t3D{{.*}}.read(
        // METALLIB: call {{.*}}.read_texture_3d.v4f32(
        && all(Tv(1) == t3D.Load(int4(0, 0, 0, 0)))

        // METAL: t1DArray{{.*}}.read(
        // METALLIB: call {{.*}}.read_texture_1d_array.v4f32(
        && all(Tv(1) == t1DArray.Load(int3(0, 0, 0)))

        // METAL: t2DArray{{.*}}.read(
        // METALLIB: call {{.*}}.read_texture_2d_array.v4f32(
        && all(Tv(1) == t2DArray.Load(int4(0, 0, 0, 0)))

        // Offset variant

#if defined(NEED_TO_TEST_FOR_METAL_CAPABILITY)
        // Metal doesn't support offset variants for Load
        && all(Tv(1) == t1D.Load(int2(0, 0), 0))
        && all(Tv(1) == t2D.Load(int3(0, 0, 0), int2(0,0)))
        && all(Tv(1) == t3D.Load(int4(0, 0, 0, 0), int3(0, 0, 0)))
        && all(Tv(1) == t1DArray.Load(int3(0, 0, 0), 0))
        && all(Tv(1) == t2DArray.Load(int4(0, 0, 0, 0), int2(0, 0)))
#endif
        ;

    return result;
}


[numthreads(1, 1, 1)]
void computeMain()
{
    // SPIR: OpEntryPoint
    // HLSL: void computeMain(

    bool result = true
        && TEST_texture_float()
        ;

    // FUNCTIONAL: 1
    outputBuffer[0] = int(result);
}
