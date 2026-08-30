//TEST:SIMPLE(filecheck=CHECK_GLSL): -Wno-120 -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV): -Wno-120 -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl -Xslang -Wno-118
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl -Xslang -Wno-118 -emit-spirv-directly

#version 460

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 4) in;

bool testAnyInvocation()
{
    return true
        && anyInvocation(gl_GlobalInvocationID.x == 0)
        && anyInvocation(gl_GlobalInvocationID.x == 8) == false
        ;
}
bool testAllInvocations()
{
    return true
        && allInvocations(true)
        && allInvocations(gl_GlobalInvocationID.x == 0) == false
        ;
}
bool testAllInvocationsEqual()
{
    return true
        && allInvocationsEqual(false)
        && allInvocationsEqual(gl_GlobalInvocationID.x == 0) == false
        ;
}

[shader("compute")]
void computeMain()
{
    outputBuffer.data[0] = true
        && testAnyInvocation()
        && testAllInvocations()
        && testAllInvocationsEqual()
        ;
    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // BUF: 1
}