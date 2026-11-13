// GLSL-source compute shader whose entry point has no `[shader("compute")]` attribute and
// sets the thread-group size only through `layout(local_size_x = 16) in;`. render-test loads
// the module and then finds the entry point with `findAndCheckEntryPoint`, so the size has to
// reach the code for the entry point even though the module was lowered before the entry
// point was known. Each thread writes its own slot and bumps a counter, so the test only
// passes when all 16 threads of the group run. The buffer is printed in hexadecimal.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl

#version 450

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
buffer OutputBlock
{
    uint data[];
} outputBuffer;

layout(local_size_x = 16) in;

void computeMain()
{
    outputBuffer.data[gl_LocalInvocationID.x] = 100u + gl_LocalInvocationID.x;
    atomicAdd(outputBuffer.data[16], 1u);
}

// BUF: 64
// BUF-NEXT: 65
// BUF-NEXT: 66
// BUF-NEXT: 67
// BUF-NEXT: 68
// BUF-NEXT: 69
// BUF-NEXT: 6A
// BUF-NEXT: 6B
// BUF-NEXT: 6C
// BUF-NEXT: 6D
// BUF-NEXT: 6E
// BUF-NEXT: 6F
// BUF-NEXT: 70
// BUF-NEXT: 71
// BUF-NEXT: 72
// BUF-NEXT: 73
// BUF-NEXT: 10
