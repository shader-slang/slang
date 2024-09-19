//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture1D(size=4, content = zero):name t1D_f32v3
Texture1D<float3> t1D_f32v3;
//TEST_INPUT: Texture2D(size=4, content = zero):name t2D_f32v3
Texture2D<float3> t2D_f32v3;
//TEST_INPUT: Texture3D(size=4, content = zero):name t3D_f32v3
Texture3D<float3> t3D_f32v3;
//TEST_INPUT: TextureCube(size=4, content = zero):name tCube_f32v3
TextureCube<float3> tCube_f32v3;
//TEST_INPUT: Texture1D(size=4, content = zero, arrayLength=2):name t1DArray_f32v3
Texture1DArray<float3> t1DArray_f32v3;
//TEST_INPUT: Texture2D(size=4, content = zero, arrayLength=2):name t2DArray_f32v3
Texture2DArray<float3> t2DArray_f32v3;
//TEST_INPUT: TextureCube(size=4, content = zero, arrayLength=2):name tCubeArray_f32v3
TextureCubeArray<float3> tCubeArray_f32v3;

//TEST_INPUT: Texture1D(size=4, content = zero):name t1D_f32
Texture1D<float4> t1D_f32;
//TEST_INPUT: Texture2D(size=4, content = zero):name t2D_f32
Texture2D<float4> t2D_f32;
//TEST_INPUT: Texture3D(size=4, content = zero):name t3D_f32
Texture3D<float4> t3D_f32;
//TEST_INPUT: TextureCube(size=4, content = zero):name tCube_f32
TextureCube<float4> tCube_f32;

//TEST_INPUT: Texture1D(size=4, content = zero, arrayLength=2):name t1DArray_f32
Texture1DArray<float4> t1DArray_f32;
//TEST_INPUT: Texture2D(size=4, content = zero, arrayLength=2):name t2DArray_f32
Texture2DArray<float4> t2DArray_f32;
//TEST_INPUT: TextureCube(size=4, content = zero, arrayLength=2):name tCubeArray_f32
TextureCubeArray<float4> tCubeArray_f32;

//TEST_INPUT: Texture1D(size=4, content = zero):name t1D_f16
Texture1D<half4> t1D_f16;
//TEST_INPUT: Texture2D(size=4, content = zero):name t2D_f16
Texture2D<half4> t2D_f16;
//TEST_INPUT: Texture3D(size=4, content = zero):name t3D_f16
Texture3D<half4> t3D_f16;
//TEST_INPUT: TextureCube(size=4, content = zero):name tCube_f16
TextureCube<half4> tCube_f16;

//TEST_INPUT: Texture1D(size=4, content = zero, arrayLength=2):name t1DArray_f16
Texture1DArray<half4> t1DArray_f16;
//TEST_INPUT: Texture2D(size=4, content = zero, arrayLength=2):name t2DArray_f16
Texture2DArray<half4> t2DArray_f16;
//TEST_INPUT: TextureCube(size=4, content = zero, arrayLength=2):name tCubeArray_f16
TextureCubeArray<half4> tCubeArray_f16;

//TEST_INPUT: Texture1D(size=4, content = zero):name t1D_i32
Texture1D<int4> t1D_i32;
//TEST_INPUT: Texture2D(size=4, content = zero):name t2D_i32
Texture2D<int4> t2D_i32;
//TEST_INPUT: Texture3D(size=4, content = zero):name t3D_i32
Texture3D<int4> t3D_i32;
//TEST_INPUT: TextureCube(size=4, content = zero):name tCube_i32
TextureCube<int4> tCube_i32;

//TEST_INPUT: Texture1D(size=4, content = zero, arrayLength=2):name t1DArray_i32
Texture1DArray<int4> t1DArray_i32;
//TEST_INPUT: Texture2D(size=4, content = zero, arrayLength=2):name t2DArray_i32
Texture2DArray<int4> t2DArray_i32;
//TEST_INPUT: TextureCube(size=4, content = zero, arrayLength=2):name tCubeArray_i32
TextureCubeArray<int4> tCubeArray_i32;

//TEST_INPUT: Texture1D(size=4, content = zero):name t1D_u32
Texture1D<uint4> t1D_u32;
//TEST_INPUT: Texture2D(size=4, content = zero):name t2D_u32
Texture2D<uint4> t2D_u32;
//TEST_INPUT: Texture3D(size=4, content = zero):name t3D_u32
Texture3D<uint4> t3D_u32;
//TEST_INPUT: TextureCube(size=4, content = zero):name tCube_u32
TextureCube<uint4> tCube_u32;

//TEST_INPUT: Texture1D(size=4, content = zero, arrayLength=2):name t1DArray_u32
Texture1DArray<uint4> t1DArray_u32;
//TEST_INPUT: Texture2D(size=4, content = zero, arrayLength=2):name t2DArray_u32
Texture2DArray<uint4> t2DArray_u32;
//TEST_INPUT: TextureCube(size=4, content = zero, arrayLength=2):name tCubeArray_u32
TextureCubeArray<uint4> tCubeArray_u32;

//TEST_INPUT: Texture1D(size=4, content = zero):name t1D_i16
Texture1D<int16_t4> t1D_i16;
//TEST_INPUT: Texture2D(size=4, content = zero):name t2D_i16
Texture2D<int16_t4> t2D_i16;
//TEST_INPUT: Texture3D(size=4, content = zero):name t3D_i16
Texture3D<int16_t4> t3D_i16;
//TEST_INPUT: TextureCube(size=4, content = zero):name tCube_i16
TextureCube<int16_t4> tCube_i16;

//TEST_INPUT: Texture1D(size=4, content = zero, arrayLength=2):name t1DArray_i16
Texture1DArray<int16_t4> t1DArray_i16;
//TEST_INPUT: Texture2D(size=4, content = zero, arrayLength=2):name t2DArray_i16
Texture2DArray<int16_t4> t2DArray_i16;
//TEST_INPUT: TextureCube(size=4, content = zero, arrayLength=2):name tCubeArray_i16
TextureCubeArray<int16_t4> tCubeArray_i16;

//TEST_INPUT: Texture1D(size=4, content = zero):name t1D_u16
Texture1D<uint16_t4> t1D_u16;
//TEST_INPUT: Texture2D(size=4, content = zero):name t2D_u16
Texture2D<uint16_t4> t2D_u16;
//TEST_INPUT: Texture3D(size=4, content = zero):name t3D_u16
Texture3D<uint16_t4> t3D_u16;
//TEST_INPUT: TextureCube(size=4, content = zero):name tCube_u16
TextureCube<uint16_t4> tCube_u16;

//TEST_INPUT: Texture1D(size=4, content = zero, arrayLength=2):name t1DArray_u16
Texture1DArray<uint16_t4> t1DArray_u16;
//TEST_INPUT: Texture2D(size=4, content = zero, arrayLength=2):name t2DArray_u16
Texture2DArray<uint16_t4> t2DArray_u16;
//TEST_INPUT: TextureCube(size=4, content = zero, arrayLength=2):name tCubeArray_u16
TextureCubeArray<uint16_t4> tCubeArray_u16;

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

//TEST_INPUT: Texture2D(size=4, content = zero):name d2D
depth2d<float> d2D;
//TEST_INPUT: TextureCube(size=4, content = zero):name dCube
depthcube<float> dCube;
//TEST_INPUT: Texture2D(size=4, content = zero, arrayLength=2):name d2DArray
depth2d_array<float> d2DArray;
//TEST_INPUT: TextureCube(size=4, content = zero, arrayLength=2):name dCubeArray
depthcube_array<float> dCubeArray;

//TEST_INPUT: Sampler:name samplerState
SamplerState samplerState;
//TEST_INPUT: Sampler:name shadowSampler
SamplerComparisonState shadowSampler;


__generic<T:__BuiltinArithmeticType, let N:int>
bool TEST_texture(
    Texture1D<vector<T,N>> t1D,
    Texture2D<vector<T,N>> t2D,
    Texture3D<vector<T,N>> t3D,
    TextureCube<vector<T,N>> tCube,
    Texture1DArray<vector<T,N>> t1DArray,
    Texture2DArray<vector<T,N>> t2DArray,
    TextureCubeArray<vector<T,N>> tCubeArray
)
{
    // WGSL-LABEL: TEST_texture
    typealias Tvn = vector<T,N>;
    typealias Tv4 = vector<T,4>;

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

    // WGSL doesn't support the mip level query;

    // WGSL: textureDimensions(t1D
    t1D.GetDimensions(width);
    voidResult = voidResult && (uint(4) == width);

    // WGSL: textureDimensions(t2D
    t2D.GetDimensions(width, height);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);

    // WGSL: textureDimensions(t3D
    t3D.GetDimensions(width, height, depth);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(4) == depth);

    // WGSL: textureDimensions(tCube
    tCube.GetDimensions(width, height);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);

    // WGSL: textureDimensions(t1DArray
    t1DArray.GetDimensions(width, elements);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(2) == elements);

    // WGSL: textureDimensions(t2DArray
    t2DArray.GetDimensions(width, height, elements);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(2) == elements);

    // WGSL: textureDimensions(tCubeArray
    tCubeArray.GetDimensions(width, height, elements);
    voidResult = voidResult && (uint(4) == width);
    voidResult = voidResult && (uint(4) == height);
    voidResult = voidResult && (uint(2) == elements);

    bool result = voidResult
        // ===============================
        // float CalculateLevelOfDetail()
        // ===============================
        // WGSL doesn't have a way to calculate mip-map level for the given coordinate

        // ========================================
        // float CalculateLevelOfDetailUnclamped()
        // ========================================
        // WGSL doesn't have a way to calculate mip-map level for the given coordinate

        // ===========
        // T Sample()
        // ===========

#if 0
        // WGSL: textureSample(t1D
        && all(Tvn(T(0)) == t1D.Sample(samplerState, u))

        // WGSL: textureSample(t2D
        && all(Tvn(T(0)) == t2D.Sample(samplerState, float2(u, u)))

        // WGSL: textureSample(t3D
        && all(Tvn(T(0)) == t3D.Sample(samplerState, float3(u, u, u)))

        // WGSL: textureSample(tCube
        && all(Tvn(T(0)) == tCube.Sample(samplerState, normalize(float3(u, 1 - u, u))))

        // WGSL: textureSample(t1DArray
        && all(Tvn(T(0)) == t1DArray.Sample(samplerState, float2(u, 0)))

        // WGSL: textureSample(t2DArray
        && all(Tvn(T(0)) == t2DArray.Sample(samplerState, float3(u, u, 0)))

        // WGSL: textureSample(tCubeArray
        && all(Tvn(T(0)) == tCubeArray.Sample(samplerState, float4(normalize(float3(u, 1 - u, u)), 0)))

        // Offset variant

        // Metal doesn't support Offset for 1D and Cube texture

        // WGSL: t2D{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == t2D.Sample(samplerState, float2(u, u), int2(1, 1)))

        // WGSL: t3D{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == t3D.Sample(samplerState, float3(u, u, u), int3(1, 1, 1)))

        // WGSL: t2DArray{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == t2DArray.Sample(samplerState, float3(u, u, 0), int2(1, 1)))

        // Clamp variant

        // Metal doesn't support Offset for 1D and Cube texture

        // WGSL: t2D{{.*}}.sample({{.*}} min_lod_clamp(
        && all(Tvn(T(0)) == t2D.Sample(samplerState, float2(u, u), int2(1, 1), float(1)))

        // WGSL: t3D{{.*}}.sample({{.*}} min_lod_clamp(
        && all(Tvn(T(0)) == t3D.Sample(samplerState, float3(u, u, u), int3(1, 1, 1), float(1)))

        // WGSL: t2DArray{{.*}}.sample({{.*}} min_lod_clamp(
        && all(Tvn(T(0)) == t2DArray.Sample(samplerState, float3(u, u, 0), int2(1, 1), float(1)))

        // ===============
        // T SampleBias()
        // ===============

        // Metal doesn't support Bias for 1D texture

        // WGSL: t2D{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == t2D.SampleBias(samplerState, float2(u, u), float(-1)))

        // WGSL: t3D{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == t3D.SampleBias(samplerState, float3(u, u, u), float(-1)))

        // WGSL: tCube{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == tCube.SampleBias(samplerState, normalize(float3(u, 1 - u, u)), float(-1)))

        // WGSL: t2DArray{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == t2DArray.SampleBias(samplerState, float3(u, u, 0), float(-1)))

        // WGSL: tCubeArray{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == tCubeArray.SampleBias(samplerState, float4(normalize(float3(u, 1 - u, u)), 0), float(-1)))

        // Offset variant

        // Metal doesn't support Offset for 1D and Cube texture

        // WGSL: t2D{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == t2D.SampleBias(samplerState, float2(u, u), float(-1), int2(1, 1)))

        // WGSL: t3D{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == t3D.SampleBias(samplerState, float3(u, u, u), float(-1), int3(1, 1, 1)))

        // WGSL: t2DArray{{.*}}.sample({{.*}}
        && all(Tvn(T(0)) == t2DArray.SampleBias(samplerState, float3(u, u, 0), float(-1), int2(1, 1)))

        // ===================================
        //  T SampleLevel()
        // ===================================

        // Metal doesn't support LOD for 1D texture

        // WGSL: t2D{{.*}}.sample({{.*}} level(
        && all(Tvn(T(0)) == t2D.SampleLevel(samplerState, float2(u, u), 0))

        // WGSL: t3D{{.*}}.sample({{.*}} level(
        && all(Tvn(T(0)) == t3D.SampleLevel(samplerState, float3(u, u, u), 0))

        // WGSL: tCube{{.*}}.sample({{.*}} level(
        && all(Tvn(T(0)) == tCube.SampleLevel(samplerState, normalize(float3(u, 1 - u, u)), 0))

        // WGSL: t2DArray{{.*}}.sample({{.*}} level(
        && all(Tvn(T(0)) == t2DArray.SampleLevel(samplerState, float3(u, u, 0), 0))

        // WGSL: tCubeArray{{.*}}.sample({{.*}} level(
        && all(Tvn(T(0)) == tCubeArray.SampleLevel(samplerState, float4(normalize(float3(u, 1 - u, u)), 0), 0))

        // Offset variant

        // Metal doesn't support LOD for 1D texture

        // WGSL: t2D{{.*}}.sample({{.*}} level(
        && all(Tvn(T(0)) == t2D.SampleLevel(samplerState, float2(u, u), 0, int2(1, 1)))

        // WGSL: t3D{{.*}}.sample({{.*}} level(
        && all(Tvn(T(0)) == t3D.SampleLevel(samplerState, float3(u, u, u), 0, int3(1, 1, 1)))

        // WGSL: t2DArray{{.*}}.sample({{.*}} level(
        && all(Tvn(T(0)) == t2DArray.SampleLevel(samplerState, float3(u, u, 0), 0, int2(1, 1)))

        // ==================
        // float SampleCmp()
        // ==================

        // WGSL: d2D{{.*}}.sample_compare(
        && float(0) == d2D.SampleCmp(shadowSampler, float2(u, u), 0)

        // WGSL: d2DArray{{.*}}.sample_compare(
        && float(0) == d2DArray.SampleCmp(shadowSampler, float3(u, u, 0), 0)

        // WGSL: dCube{{.*}}.sample_compare(
        && float(0) == dCube.SampleCmp(shadowSampler, normalize(float3(u, 1 - u, u)), 0)

        // WGSL: dCubeArray{{.*}}.sample_compare(
        && float(0) == dCubeArray.SampleCmp(shadowSampler, float4(normalize(float3(u, 1 - u, u)), 0), 0)

        // Offset variant

        // WGSL: d2D{{.*}}.sample_compare(
        && float(0) == d2D.SampleCmp(shadowSampler, float2(u2, u), 0, int2(0, 0))

        // ===================================
        //  float SampleCmpLevelZero()
        // ===================================

        // WGSL: d2D{{.*}}.sample_compare(
        && float(0) == d2D.SampleCmpLevelZero(shadowSampler, float2(u, u), 0)

        // WGSL: d2DArray{{.*}}.sample_compare(
        && float(0) == d2DArray.SampleCmpLevelZero(shadowSampler, float3(u, u, 0), 0)

        // WGSL: dCube{{.*}}.sample_compare(
        && float(0) == dCube.SampleCmpLevelZero(shadowSampler, normalize(float3(u, 1 - u, u)), 0)

        // WGSL: dCubeArray{{.*}}.sample_compare(
        && float(0) == dCubeArray.SampleCmpLevelZero(shadowSampler, float4(normalize(float3(u, 1-u, u)), 0), 0)

        // Offset variant

        // WGSL: d2D{{.*}}.sample_compare(
        && float(0) == d2D.SampleCmpLevelZero(shadowSampler, float2(u2, u), 0, int2(0, 0))

        // ==================================
        //  vector<T,4> Gather()
        // ==================================

        // WGSL: t2D{{.*}}.gather(
        && all(Tv4(T(0)) == t2D.Gather(samplerState, float2(u, u)))

        // WGSL: tCube{{.*}}.gather(
        && all(Tv4(T(0)) == tCube.Gather(samplerState, normalize(float3(u, 1 - u, u))))

        // WGSL: t2DArray{{.*}}.gather(
        && all(Tv4(T(0)) == t2DArray.Gather(samplerState, float3(u, u, 0)))

        // WGSL: tCubeArray{{.*}}.gather(
        && all(Tv4(T(0)) == tCubeArray.Gather(samplerState, float4(normalize(float3(u, 1 - u, u)), 0)))

        // Offset variant

        // WGSL: t2D{{.*}}.gather(
        && all(Tv4(T(0)) == t2D.Gather(samplerState, float2(u2, u), int2(0, 0)))

        // WGSL: t2DArray{{.*}}.gather(
        && all(Tv4(T(0)) == t2DArray.Gather(samplerState, float3(u2, u, 0), int2(0, 0)))

        // =====================================
        //  T SampleGrad()
        // =====================================

        // Metal doesn't support LOD for 1D texture

        // WGSL: t2D{{.*}}.sample(
        && all(Tvn(T(0)) == t2D.SampleGrad(samplerState, float2(u, u), float2(ddx, ddx), float2(ddy, ddy)))

        // WGSL: t3D{{.*}}.sample(
        && all(Tvn(T(0)) == t3D.SampleGrad(samplerState, float3(u, u, u), float3(ddx, ddx, ddx), float3(ddy, ddy, ddy)))

        // WGSL: tCube{{.*}}.sample(
        && all(Tvn(T(0)) == tCube.SampleGrad(samplerState, normalize(float3(u, 1 - u, u)), float3(ddx, ddx, ddx), float3(ddy, ddy, ddy)))

        // WGSL: t2DArray{{.*}}.sample(
        && all(Tvn(T(0)) == t2DArray.SampleGrad(samplerState, float3(u, u, 0.0f), float2(ddx, ddx), float2(ddy, ddy)))

        // Offset variant

        // WGSL: t2D{{.*}}.sample(
        && all(Tvn(T(0)) == t2D.SampleGrad(samplerState, float2(u2, u), float2(ddx, ddx), float2(ddy, ddy), int2(0, 0)))

        // WGSL: t3D{{.*}}.sample(
        && all(Tvn(T(0)) == t3D.SampleGrad(samplerState, float3(u2, u, u), float3(ddx, ddx, ddx), float3(ddy, ddy, ddy), int3(0, 0, 0)))

        // WGSL: t2DArray{{.*}}.sample(
        && all(Tvn(T(0)) == t2DArray.SampleGrad(samplerState, float3(u2, u, 0.0f), float2(ddx, ddx), float2(ddy, ddy), int2(0, 0)))

        // ===================
        //  T Load()
        // ===================

        // WGSL: t1D{{.*}}.read(
        && all(Tvn(T(0)) == t1D.Load(int2(0, 0)))

        // WGSL: t2D{{.*}}.read(
        && all(Tvn(T(0)) == t2D.Load(int3(0, 0, 0)))

        // WGSL: t3D{{.*}}.read(
        && all(Tvn(T(0)) == t3D.Load(int4(0, 0, 0, 0)))

        // WGSL: t1DArray{{.*}}.read(
        && all(Tvn(T(0)) == t1DArray.Load(int3(0, 0, 0)))

        // WGSL: t2DArray{{.*}}.read(
        && all(Tvn(T(0)) == t2DArray.Load(int4(0, 0, 0, 0)))

        // Offset variant

        // Metal doesn't support offset variants for Load
#endif
        ;

    return result;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    bool result = true
        && TEST_texture<float, 3>(
            t1D_f32v3,
            t2D_f32v3,
            t3D_f32v3,
            tCube_f32v3,
            t1DArray_f32v3,
            t2DArray_f32v3,
            tCubeArray_f32v3)
#if 0
        && TEST_texture<float, 4>(
            t1D_f32,
            t2D_f32,
            t3D_f32,
            tCube_f32,
            t1DArray_f32,
            t2DArray_f32,
            tCubeArray_f32)
#if !defined(EXCLUDE_HALF_TYPE)
        && TEST_texture<half, 4>(
            t1D_f16,
            t2D_f16,
            t3D_f16,
            tCube_f16,
            t1DArray_f16,
            t2DArray_f16,
            tCubeArray_f16)
#endif
#if !defined(EXCLUDE_INTEGER_TYPE)
        && TEST_texture<int, 4>(
            t1D_i32,
            t2D_i32,
            t3D_i32,
            tCube_i32,
            t1DArray_i32,
            t2DArray_i32,
            tCubeArray_i32)
        && TEST_texture<uint, 4>(
            t1D_u32,
            t2D_u32,
            t3D_u32,
            tCube_u32,
            t1DArray_u32,
            t2DArray_u32,
            tCubeArray_u32)
#if !defined(EXCLUDE_SHORT_TYPE)
        && TEST_texture<int16_t, 4>(
            t1D_i16,
            t2D_i16,
            t3D_i16,
            tCube_i16,
            t1DArray_i16,
            t2DArray_i16,
            tCubeArray_i16)
        && TEST_texture<uint16_t, 4>(
            t1D_u16,
            t2D_u16,
            t3D_u16,
            tCube_u16,
            t1DArray_u16,
            t2DArray_u16,
            tCubeArray_u16)
#endif
#endif
#endif
        ;

    // FUNCTIONAL: 1
    outputBuffer[0] = int(result);
}
