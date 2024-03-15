//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 1) in;

//TEST_INPUT: RWTextureBuffer(format=R32_UINT, stride=4, data=[1]):name i_ib
uniform layout(binding=0,r32ui) coherent uimageBuffer i_ib;

// CHECK_GLSL: void main(
// CHECK_SPV: OpEntryPoint
void computeMain()
{
    outputBuffer.data[0] = (imageLoad(i_ib,0).x);

    imageAtomicAdd(i_ib, 0, 0x2);

    outputBuffer.data[1] = int(imageLoad(i_ib,0).x);
    // BUF: 1
    // BUF: 3
}
