// Regression for https://github.com/shader-slang/slang/issues/11731
// A non-relaxed atomic on an SSBO member lowers to the legacy `Uniform`
// storage class when targeting SPIR-V < 1.4 (via -profile glsl_460). The
// emitted Memory Semantics operand must therefore OR in the UniformMemory
// storage-class bit, otherwise Vulkan rejects the module with
// VUID-StandaloneSpirv-MemorySemantics-10870.
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
    // AcquireRelease(0x8) | UniformMemory(0x40) == 0x48 == 72
    // CHECK: OpAtomicIAdd {{.*}} %uint_72
    __atomic_add(data, 1u, MemoryOrder::AcquireRelease);
}
