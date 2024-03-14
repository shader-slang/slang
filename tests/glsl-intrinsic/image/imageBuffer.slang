//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 4) in;

//TEST_INPUT: RWTextureBuffer(format=R16G16B16A16_UINT, stride=8, data=[0x00010001 0x00010001 0x00010001 0x00010001]):name i_ib
uniform layout(binding=0,rgba16ui) readonly writeonly uimageBuffer i_ib;

// CHECK_GLSL: void main(
// CHECK_SPV: OpEntryPoint
void computeMain()
{
    outputBuffer.data[0] = uint(imageLoad(i_ib,0).x);
        ;
    // BUF: 1
}
