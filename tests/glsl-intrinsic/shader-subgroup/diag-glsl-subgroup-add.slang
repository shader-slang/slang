// Diagnostic test: verify that subgroupAdd counts the correct number of
// active threads when using GLSL layout(local_size_x) on Metal.
//
// subgroupAdd(1) should return the number of active threads in the subgroup.
// With local_size_x = 4, thread 0 should see at least 4 (could be higher
// if subgroup size > workgroup size, but never 1).
//
// If dispatch only launches 1 thread, subgroupAdd(1) returns 1.
//
// See https://github.com/shader-slang/slang/issues/9953

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -allow-glsl

#version 430

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 4) in;

void computeMain()
{
    uint tid = gl_LocalInvocationIndex;

    // All threads participate in the collective operation
    uint activeCount = subgroupAdd(uint(1));

    if (tid == 0) {
        outputBuffer.data[0] = gl_SubgroupSize;
        outputBuffer.data[1] = activeCount;
        // Write 1 if activeCount >= 4 (workgroup size), 0 otherwise
        outputBuffer.data[2] = (activeCount >= uint(4)) ? uint(1) : uint(0);
    }

    // slot 2: must be 1 (activeCount >= 4)
    // BUF: {{.*}}
    // BUF-NEXT: {{.*}}
    // BUF-NEXT: 1
}
