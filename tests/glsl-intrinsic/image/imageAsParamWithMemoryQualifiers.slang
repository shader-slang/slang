//TEST:SIMPLE_EX(filecheck=CHECK_GLSL): -lang glsl tests/glsl-intrinsic/image/imageAsParamWithMemoryQualifiers.slang -stage compute -entry computeMain -target glsl -Wno-118
//TEST:SIMPLE_EX(filecheck=CHECK_SPV): -lang glsl tests/glsl-intrinsic/image/imageAsParamWithMemoryQualifiers.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly -Wno-118
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl -Xslang -Wno-118
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl -emit-spirv-directly -Xslang -Wno-118

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[1];
} outputBuffer;

layout(local_size_x = 4) in;

// The memory qualifier `restrict` is allowed to be dropped
// when passing a variable into a function as an argument.

//CHECK_SPV: OpEntryPoint

//CHECK_GLSL: {{(restrict|writeonly|readonly)}} {{(restrict|writeonly|readonly)}} {{(restrict|writeonly|readonly)}}
//CHECK_SPV: OpDecorate %{{.*}} {{(Restrict|NonWritable|NonReadable)}}
//CHECK_SPV: OpDecorate %{{.*}} {{(Restrict|NonWritable|NonReadable)}}
//CHECK_SPV: OpDecorate %{{.*}} {{(Restrict|NonWritable|NonReadable)}}
//TEST_INPUT: set someImage = RWTexture2D(format=RGBA16Float, size=1, content=one, mipMaps = 1)
uniform layout(binding=0,rgba16f) restrict writeonly readonly image2D someImage;

//CHECK_GLSL: writeonly
//CHECK_SPV: OpDecorate {{.*}} {{(Restrict|NonWritable|NonReadable)}}
//TEST_INPUT: set someImage2 = RWTexture2D(format=RGBA16Float, size=1, content=one, mipMaps = 1)
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
void computeMain()
{
    outputBuffer.data[0] = true
        && checkAllImageSizesParamSameQualifiers(someImage)
        && checkAllImageSizesParamMoreQualifiers(someImage2)
        ;
    // BUF: 1

}
