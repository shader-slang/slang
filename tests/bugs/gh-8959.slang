// Repro for https://github.com/shader-slang/slang/issues/8959
// Error 41403 incorrectly fires for atomics on GLSL SSBO members.
// In SPIRV 1.3, SSBOs use Uniform storage class with BufferBlock decoration
// (not StorageBuffer), so isValidAtomicDest must accept that address space.
//TEST:SIMPLE(filecheck=CHECK):-profile glsl_460 -target spirv

//CHECK-NOT: E41403

#version 460

layout(std430, binding = 0) buffer testBuffer
{
    uint data;
}

RWStructuredBuffer<uint> validBuf;
RWBuffer<uint> typedBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // SSBO member atomics — all three should compile without error 41403.
    __atomic_add(data, 1u, MemoryOrder::AcquireRelease);
    InterlockedAdd(data, 1);
    atomicAdd(data, 1u);

    // RWStructuredBuffer and RWBuffer atomics via __ref-based functions.
    uint ignored;
    InterlockedAdd(validBuf[0], 1, ignored);
    InterlockedAdd(typedBuf[0], 1, ignored);
    __atomic_add(typedBuf[0], 1u, MemoryOrder::Relaxed);
}
