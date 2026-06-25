// Regression for https://github.com/shader-slang/slang/issues/11731.
//TEST:SIMPLE(filecheck=CHECK):-profile glsl_460 -target spirv

#version 460

layout(std430, binding = 0) buffer testBuffer
{
    uint data;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // CHECK: OpDecorate {{.*}} BufferBlock
    // CHECK: OpAtomicIAdd {{.*}} %uint_72
    __atomic_add(data, 1u, MemoryOrder::AcquireRelease);
}
