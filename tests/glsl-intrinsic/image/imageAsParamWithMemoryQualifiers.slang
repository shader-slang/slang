//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[1];
} outputBuffer;

layout(local_size_x = 4) in;

//TEST_INPUT: RWTexture2D(format=R16G16B16A16_FLOAT, size=1, content=one, mipMaps = 1):name someImage
uniform layout(binding=0,rgba16f) writeonly readonly image2D someImage;

//TEST_INPUT: RWTexture2D(format=R16G16B16A16_FLOAT, size=1, content=one, mipMaps = 1):name someImage2
uniform layout(binding=1,rgba16f) writeonly image2D someImage2;

bool checkAllImageSizesParamSameQualifiers(writeonly readonly image2D val)
{
    return true
        && imageSize(val) == ivec2(1)
        ;
}
bool checkAllImageSizesParamMoreQualifiers(readonly writeonly image2D val)
{
    return true
        && imageSize(val) == ivec2(1)
        ;
}

// CHECK_GLSL: void main(
// CHECK_SPV: OpEntryPoint
void computeMain()
{
    outputBuffer.data[0] = true
        && checkAllImageSizesParamSameQualifiers(someImage)
        && checkAllImageSizesParamMoreQualifiers(someImage2)
        ;
    // BUF: 1

}