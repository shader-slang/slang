// Texture types capability test

// TEST_OP
//
// sampler ops
//   1  - sample with 1-dimensional coord
//   2  - sample with 2-dimensional coord
//   3  - sample with 3-dimensional coord
//   4  - sample with 4-dimensional coord
//
// load ops, mip_level 0
//  11  - load with 1-dimensional coord
//  12  - load with 2-dimensional coord
//  13  - load with 3-dimensional coord
//  14  - load with 4-dimensional coord
//
// store ops, mip_level 0
//  21  - store with 1-dimensional coord
//  22  - store with 2-dimensional coord
//  23  - store with 3-dimensional coord
//
// store ops

// WGSL 1.0 tests
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=Texture1D<float4>          -DTEST_OP=12 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=NEGATIVE): -DTEST_TYPE=Texture1DArray<float4>     -DTEST_OP=13 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=Texture2D<float4>          -DTEST_OP=13 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=Texture2DArray<float4>     -DTEST_OP=14 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=Texture2DMS<float4>        -DTEST_OP=13 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=NEGATIVE): -DTEST_TYPE=Texture2DMSArray<float4>   -DTEST_OP=14 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=Texture3D<float4>          -DTEST_OP=14 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=TextureCube<float4>        -DTEST_OP=3  -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=TextureCubeArray<float4>   -DTEST_OP=4  -stage fragment -entry testMain -target wgsl

//TEST:SIMPLE(filecheck=NEGATIVE): -DTEST_TYPE=DepthTexture1D             -DTEST_OP=12 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=NEGATIVE): -DTEST_TYPE=DepthTexture1DArray        -DTEST_OP=13 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=DepthTexture2D             -DTEST_OP=13 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=DepthTexture2DArray        -DTEST_OP=14 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=DepthTexture2DMS           -DTEST_OP=13 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=NEGATIVE): -DTEST_TYPE=DepthTexture2DMSArray      -DTEST_OP=14 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=NEGATIVE): -DTEST_TYPE=DepthTexture3D             -DTEST_OP=14 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=DepthTextureCube           -DTEST_OP=3  -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=DepthTextureCubeArray      -DTEST_OP=4  -stage fragment -entry testMain -target wgsl

//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=RWTexture1D<float4>        -DTEST_OP=11 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=NEGATIVE): -DTEST_TYPE=RWTexture1DArray<float4>   -DTEST_OP=12 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=RWTexture2D<float4>        -DTEST_OP=12 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=RWTexture2DArray<float4>   -DTEST_OP=13 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=NEGATIVE): -DTEST_TYPE=RWTexture2DMS<float4>      -DTEST_OP=12 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=NEGATIVE): -DTEST_TYPE=RWTexture2DMSArray<float4> -DTEST_OP=13 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=RWTexture3D<float4>        -DTEST_OP=13 -stage fragment -entry testMain -target wgsl

//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=WTexture1D<float4>         -DTEST_OP=21 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=NEGATIVE): -DTEST_TYPE=WTexture1DArray<float4>    -DTEST_OP=22 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=WTexture2D<float4>         -DTEST_OP=22 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=WTexture2DArray<float4>    -DTEST_OP=23 -stage fragment -entry testMain -target wgsl
//TEST:SIMPLE(filecheck=POSITIVE): -DTEST_TYPE=WTexture3D<float4>         -DTEST_OP=23 -stage fragment -entry testMain -target wgsl

// TODO: feedback textures, rasterizer ordered textures


//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture1D(size=4, content = zero):name t1D_f32v4
TEST_TYPE  texHandle;

SamplerState samplerState;

void testMain()
{
// POSITIVE: result code = 0
// POSITIVE-LABEL: testMain
// POSITIVE-NOT: error

// NEGATIVE: result code = -1
// NEGATIVE: error

#if TEST_OP == 1
    outputBuffer[0] = int((texHandle.Sample(samplerState, float(0))).x);
#elif TEST_OP == 2
    outputBuffer[0] = int((texHandle.Sample(samplerState, float2(0, 0))).x);
#elif TEST_OP == 3
    outputBuffer[0] = int((texHandle.Sample(samplerState, float3(0, 0, 0))).x);
#elif TEST_OP == 4
    outputBuffer[0] = int((texHandle.Sample(samplerState, float4(0, 0, 0, 0))).x);
#elif TEST_OP == 11
    outputBuffer[0] = int((texHandle.Load(int(0))).x);
#elif TEST_OP == 12
    outputBuffer[0] = int((texHandle.Load(int2(0, 0))).x);
#elif TEST_OP == 13
    outputBuffer[0] = int((texHandle.Load(int3(0, 0, 0))).x);
#elif TEST_OP == 14
    outputBuffer[0] = int((texHandle.Load(int4(0, 0, 0, 0))).x);
#elif TEST_OP == 21
    texHandle.Store(int(0), float4(0, 0, 0, 0));
    outputBuffer[0] = 0;
#elif TEST_OP == 22
    texHandle.Store(int2(0, 0), float4(0, 0, 0, 0));
    outputBuffer[0] = 0;
#elif TEST_OP == 23
    texHandle.Store(int3(0, 0, 0), float4(0, 0, 0, 0));
    outputBuffer[0] = 0;
#else
#error Unimplemented test op
#endif

}
