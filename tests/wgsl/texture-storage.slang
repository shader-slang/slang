//TEST:SIMPLE(filecheck=WGSL): -stage fragment -entry fragMain -target wgsl

// In WGSL, `textureSample` and other variants work only for f32 type.
// But textureLoad works for i32, u32 and f32.

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// f32 types

//TEST_INPUT: RWTexture1D(format=RG32Float, size=4, content = zero):name w1D_f32v2
//TEST_INPUT: RWTexture2D(format=RG32Float, size=4, content = zero):name w2D_f32v2
//TEST_INPUT: RWTexture3D(format=RG32Float, size=4, content = zero):name w3D_f32v2
//TEST_INPUT: RWTexture2D(format=RG32Float, size=4, content = zero, arrayLength=2):name w2DArray_f32v2
// WGSL: var w1D_f32v2{{[^:]*}}: texture_storage_1d<rg32float, read_write>
RWTexture1D<rg32f>      w1D_f32v2;
RWTexture2D<rg32f>      w2D_f32v2;
RWTexture3D<rg32f>      w3D_f32v2;
RWTexture2DArray<rg32f> w2DArray_f32v2;

//TEST_INPUT: RWTexture1D(format=RGBA8Unorm, size=4, content = zero):name w1D_f32v4
//TEST_INPUT: RWTexture2D(format=RGBA8Unorm, size=4, content = zero):name w2D_f32v4
//TEST_INPUT: RWTexture3D(format=RGBA8Unorm, size=4, content = zero):name w3D_f32v4
//TEST_INPUT: RWTexture2D(format=RGBA8Unorm, size=4, content = zero, arrayLength=2):name w2DArray_f32v4
// WGSL: var w1D_f32v4{{[^:]*}}: texture_storage_1d<rgba8unorm, read_write>
RWTexture1D<rgba8>      w1D_f32v4;
RWTexture2D<rgba8>      w2D_f32v4;
RWTexture3D<rgba8>      w3D_f32v4;
RWTexture2DArray<rgba8> w2DArray_f32v4;

//TEST_INPUT: RWTexture1D(format=BGRA8Unorm, size=4, content = zero):name w1D_f32_bgra_v4
//TEST_INPUT: RWTexture2D(format=BGRA8Unorm, size=4, content = zero):name w2D_f32_bgra_v4
//TEST_INPUT: RWTexture3D(format=BGRA8Unorm, size=4, content = zero):name w3D_f32_bgra_v4
//TEST_INPUT: RWTexture2D(format=BGRA8Unorm, size=4, content = zero, arrayLength=2):name w2DArray_f32_bgra_v4
// WGSL: var w1D_f32_bgra_v4{{[^:]*}}: texture_storage_1d<bgra8unorm, read_write>
RWTexture1D<bgra8>      w1D_f32_bgra_v4;
RWTexture2D<bgra8>      w2D_f32_bgra_v4;
RWTexture3D<bgra8>      w3D_f32_bgra_v4;
RWTexture2DArray<bgra8> w2DArray_f32_bgra_v4;

// i32 types

//TEST_INPUT: RWTexture1D(format=RG32Sint, size=4, content = zero):name w1D_i32v2
//TEST_INPUT: RWTexture2D(format=RG32Sint, size=4, content = zero):name w2D_i32v2
//TEST_INPUT: RWTexture3D(format=RG32Sint, size=4, content = zero):name w3D_i32v2
//TEST_INPUT: RWTexture2D(format=RG32Sint, size=4, content = zero, arrayLength=2):name w2DArray_i32v2
// WGSL: var w1D_i32v2{{[^:]*}}: texture_storage_1d<rg32sint, read_write>
RWTexture1D<rg32i>      w1D_i32v2;
RWTexture2D<rg32i>      w2D_i32v2;
RWTexture3D<rg32i>      w3D_i32v2;
RWTexture2DArray<rg32i> w2DArray_i32v2;

//TEST_INPUT: RWTexture1D(format=RGBA32Sint, size=4, content = zero):name w1D_i32v4
//TEST_INPUT: RWTexture2D(format=RGBA32Sint, size=4, content = zero):name w2D_i32v4
//TEST_INPUT: RWTexture3D(format=RGBA32Sint, size=4, content = zero):name w3D_i32v4
//TEST_INPUT: RWTexture2D(format=RGBA32Sint, size=4, content = zero, arrayLength=2):name w2DArray_i32v4
// WGSL: var w1D_i32v4{{[^:]*}}: texture_storage_1d<rgba32sint, read_write>
RWTexture1D<rgba32i>      w1D_i32v4;
RWTexture2D<rgba32i>      w2D_i32v4;
RWTexture3D<rgba32i>      w3D_i32v4;
RWTexture2DArray<rgba32i> w2DArray_i32v4;

// u32 types

//TEST_INPUT: RWTexture1D(format=RG32Uint, size=4, content = zero):name w1D_u32v2
//TEST_INPUT: RWTexture2D(format=RG32Uint, size=4, content = zero):name w2D_u32v2
//TEST_INPUT: RWTexture3D(format=RG32Uint, size=4, content = zero):name w3D_u32v2
//TEST_INPUT: RWTexture2D(format=RG32Uint, size=4, content = zero, arrayLength=2):name w2DArray_u32v2
// WGSL: var w1D_u32v2{{[^:]*}}: texture_storage_1d<rg32uint, read_write>
RWTexture1D<rg32ui>      w1D_u32v2;
RWTexture2D<rg32ui>      w2D_u32v2;
RWTexture3D<rg32ui>      w3D_u32v2;
RWTexture2DArray<rg32ui> w2DArray_u32v2;

//TEST_INPUT: RWTexture1D(format=RGBA16Uint, size=4, content = zero):name w1D_u32v4
//TEST_INPUT: RWTexture2D(format=RGBA16Uint, size=4, content = zero):name w2D_u32v4
//TEST_INPUT: RWTexture3D(format=RGBA16Uint, size=4, content = zero):name w3D_u32v4
//TEST_INPUT: RWTexture2D(format=RGBA16Uint, size=4, content = zero, arrayLength=2):name w2DArray_u32v4
// WGSL: var w1D_u32v4{{[^:]*}}: texture_storage_1d<rgba16uint, read_write>
RWTexture1D<rgba16ui>      w1D_u32v4;
RWTexture2D<rgba16ui>      w2D_u32v4;
RWTexture3D<rgba16ui>      w3D_u32v4;
RWTexture2DArray<rgba16ui> w2DArray_u32v4;


__generic<T : ITexelElement>
[ForceInline] // Workaround for a WGSL requirement that `texture_storage_Xd` must always be a global variable
bool TEST_textureStorage_StoreLoad(
    RWTexture1D<T> w1D,
    RWTexture2D<T> w2D,
    RWTexture3D<T> w3D,
    RWTexture2DArray<T> w2DArray)
    where T.DataType : IArithmetic
{
    typealias Tvn = T.DataType;

    // ===================
    // o[i] = v;
    // https://www.w3.org/TR/WGSL/#texturestore
    // ===================

    // TODO: store before load
    // WGSL: textureStore({{\(*}}w1D
    w1D[0] = Tvn(T.DataType.Element(1));

    // WGSL: textureStore({{\(*}}w2D
    w2D[0] = Tvn(T.DataType.Element(1));

    // WGSL: textureStore({{\(*}}w3D
    w3D[0] = Tvn(T.DataType.Element(1));

    // WGSL: textureStore({{\(*}}w2DArray
    w2DArray[0] = Tvn(T.DataType.Element(1));

    return true
        // ===================
        // T Load()
        // https://www.w3.org/TR/WGSL/#textureload
        // ===================

        // WGSL: textureLoad({{\(*}}w1D
        && all(Tvn(T.DataType.Element(0)) == w1D.Load(0))

        // WGSL: textureLoad({{\(*}}w2D
        && all(Tvn(T.DataType.Element(0)) == w2D.Load(int2(0, 0)))

        // WGSL: textureLoad({{\(*}}w3D
        && all(Tvn(T.DataType.Element(0)) == w3D.Load(int3(0, 0, 0)))

        // WGSL: textureLoad({{\(*}}w2DArray
        && all(Tvn(T.DataType.Element(0)) == w2DArray.Load(int3(0, 0, 0)))
        ;
}

void fragMain()
{
    bool result = true
        && TEST_textureStorage_StoreLoad<rg32f>(w1D_f32v2, w2D_f32v2, w3D_f32v2, w2DArray_f32v2)
        && TEST_textureStorage_StoreLoad<rgba8>(w1D_f32v4, w2D_f32v4, w3D_f32v4, w2DArray_f32v4)
        && TEST_textureStorage_StoreLoad<bgra8>(w1D_f32_bgra_v4, w2D_f32_bgra_v4, w3D_f32_bgra_v4, w2DArray_f32_bgra_v4)
        && TEST_textureStorage_StoreLoad<rg32i>(w1D_i32v2, w2D_i32v2, w3D_i32v2, w2DArray_i32v2)
        && TEST_textureStorage_StoreLoad<rgba32i>(w1D_i32v4, w2D_i32v4, w3D_i32v4, w2DArray_i32v4)
        && TEST_textureStorage_StoreLoad<rg32ui>(w1D_u32v2, w2D_u32v2, w3D_u32v2, w2DArray_u32v2)
        && TEST_textureStorage_StoreLoad<rgba16ui>(w1D_u32v4, w2D_u32v4, w3D_u32v4, w2DArray_u32v4)
        ;

    outputBuffer[0] = int(result);
}
