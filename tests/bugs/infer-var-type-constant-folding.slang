//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
//BUF: B

RWStructuredBuffer<int> outputBuffer;

static const let C = float(3);
static const let D = int(4+4+int(C));

void computeMain()
{
    outputBuffer[0] = D;
}