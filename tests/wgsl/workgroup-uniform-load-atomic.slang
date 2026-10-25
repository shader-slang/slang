// workgroupUniformLoad of a groupshared Atomic<T> maps to WGSL's workgroupUniformLoad on an
// atomic<T> (which returns T), and to an atomic load on the other targets.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
// No wgsl-spirv-asm run: the bundled slang-tint predates workgroupUniformLoad on atomics (Chrome 137,
// see #13861); current Tint accepts the output.
//TEST:SIMPLE(filecheck=VK): -target spirv -stage compute -entry computeMain

RWStructuredBuffer<uint> outputBuffer;
groupshared Atomic<uint> batchStart;

// VK: OpAtomicLoad
// WGSL: var<workgroup> batchStart_{{[0-9]+}} : atomic<u32>;
// WGSL: : u32 = (workgroupUniformLoad(&((batchStart_{{[0-9]+}}))));
[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uint gi: SV_GroupIndex)
{
    if (gi == 0)
        batchStart.add(64);
    // workgroupUniformLoad synchronizes on WGSL only; on the other targets (see #13631) it is a plain
    // atomic load, so this barrier orders the add before every invocation's load.
    GroupMemoryBarrierWithGroupSync();
    // Uniform across the workgroup, so the loop below may contain barriers.
    uint start = workgroupUniformLoad(batchStart);
    for (uint i = 0; i < start; i += 64)
    {
        GroupMemoryBarrierWithGroupSync();
        outputBuffer[tid.x] += i;
    }
}
