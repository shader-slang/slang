import "glsl";

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=OUTPUT):-vk -compute -shaderobj
//CHECK: OpExecutionMode %computeMain LocalSize 1 2 3

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1u, 2u, 3)]
void computeMain()
{
    const int x = gl_WorkGroupSize.x;
    outputBuffer[0] = x;
    // OUTPUT: 1
}
