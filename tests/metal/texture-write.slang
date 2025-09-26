//DISABLE_TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal -DEMIT_SOURCE
//DISABLE_TEST:SIMPLE(filecheck=METALLIB): -stage compute -entry computeMain -target metallib
//DISABLE_TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -output-using-type

// Test framework has issues with RWTexture2D on Metal
// TODO: github issue #8454

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float4> outputBuffer;

// for some reason, metal textures dont have an overload for less-than-four component
// writes, they need to be converted to 4-components in a legalize step, as the other components
// get discarded

//TEST_INPUT: RWTexture2D(format=R32Float, size=4, content=zero):name pWrites.tex1
//TEST_INPUT: RWTexture2D(format=RG32Float, size=4, content=zero):name pWrites.tex2
//TEST_INPUT: RWTexture2D(format=RGBA32Float, size=4, content=zero):name pWrites.tex4
//TEST_INPUT: RWTexture2D(format=R32Float, size=4, content=zero, arrayLength=2):name pWrites.tex1Array
//TEST_INPUT: RWTexture2D(format=RG32Float, size=4, content=zero, arrayLength=2):name pWrites.tex2Array
//TEST_INPUT: RWTexture2D(format=RGBA32Float, size=4, content=zero, arrayLength=2):name pWrites.tex4Array
struct TextureWrite
{
    RWTexture2D<float> tex1;
    RWTexture2D<float2> tex2;
    RWTexture2D<float4> tex4;

    RWTexture2DArray<float> tex1Array;
    RWTexture2DArray<float2> tex2Array;
    RWTexture2DArray<float4> tex4Array;
}
ParameterBlock<TextureWrite> pWrites;

[numthreads(1, 1, 1)]
void computeMain()
{
    // TODO: check for the type of first parameter to be a 4-component vector
    // METALLIB: call {{.*}}.write_texture_2d.v4f32(
    // METAL: .write(
    pWrites.tex1[uint2(0, 0)] = float(1);
    // METALLIB: call {{.*}}.write_texture_2d.v4f32(
    // METAL: .write(
    pWrites.tex2[uint2(1, 1)] = float2(1, 2);
    // METALLIB: call {{.*}}.write_texture_2d.v4f32(
    // METAL: .write(
    pWrites.tex4[uint2(3, 3)] = float4(1, 2, 3, 4);

    // METALLIB: call {{.*}}.write_texture_2d_array.v4f32(
    // METAL: .write(
    pWrites.tex1Array[uint3(0, 0, 0)] = 1;
    // METALLIB: call {{.*}}.write_texture_2d_array.v4f32(
    // METAL: .write(
    pWrites.tex2Array[uint3(1, 1, 0)] = float2(1, 2);
    // METALLIB: call {{.*}}.write_texture_2d_array.v4f32(
    // METAL: .write(
    pWrites.tex4Array[uint3(3, 3, 0)] = float4(1, 2, 3, 4);

    // BUF: 6.000000
    // BUF: 8.000000
    // BUF: 6.000000
    // BUF: 8.000000
    outputBuffer[0] = float4(0)
        + float4(pWrites.tex1[uint2(0, 0)], 0, 0, 0)
        + float4(pWrites.tex2[uint2(1, 1)], 0, 0)
        + float4(pWrites.tex4[uint2(3, 3)])
        + float4(pWrites.tex1Array[uint3(0, 0, 0)], 0, 0, 0)
        + float4(pWrites.tex2Array[uint3(1, 1, 0)], 0, 0)
        + float4(pWrites.tex4Array[uint3(3, 3, 0)])
        ;
}
