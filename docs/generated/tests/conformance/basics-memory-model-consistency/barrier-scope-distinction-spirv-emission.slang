//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#barriers
//META: doc_section_digest=28e72df67f1e8f2f68fd59f23431c2be534ec39fa4206bc57dd04922591e5594
//META: purpose=The three address-space scopes (All, Device, ThreadGroup) for memory barriers produce distinct SPIRV memory semantics operands: All=2376, Device=2120, ThreadGroup=264.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc defines three distinct address-space scopes for barriers:
// - All: device + thread group memory accesses (semantics 2376)
// - Device: address spaces with all-threads instance scope, including storage buffers (semantics 2120)
// - Thread group: thread group (groupshared) memory accesses (semantics 264)
// This test verifies the three scopes produce non-overlapping SPIRV semantics values,
// confirming the boundary between the scope definitions.

//TEST:SIMPLE(filecheck=ALLCHECK):-target spirv-asm -entry computeAllMain -stage compute
//TEST:SIMPLE(filecheck=DEVCHECK):-target spirv-asm -entry computeDevMain -stage compute
//TEST:SIMPLE(filecheck=GRPCHECK):-target spirv-asm -entry computeGrpMain -stage compute
//TEST:SIMPLE(filecheck=METALALLCHECK):-target metal -entry computeAllMain -stage compute
//TEST:SIMPLE(filecheck=METALDEVCHECK):-target metal -entry computeDevMain -stage compute
//TEST:SIMPLE(filecheck=METALGRPCHECK):-target metal -entry computeGrpMain -stage compute
//TEST:SIMPLE(filecheck=WGSLALLCHECK):-target wgsl -entry computeAllMain -stage compute
//TEST:SIMPLE(filecheck=WGSLDEVCHECK):-target wgsl -entry computeDevMain -stage compute
//TEST:SIMPLE(filecheck=WGSLGRPCHECK):-target wgsl -entry computeGrpMain -stage compute
//TEST:SIMPLE(filecheck=CUDAALLCHECK):-target cuda -entry computeAllMain -stage compute
//TEST:SIMPLE(filecheck=CUDADEVCHECK):-target cuda -entry computeDevMain -stage compute
//TEST:SIMPLE(filecheck=CUDAGRPCHECK):-target cuda -entry computeGrpMain -stage compute
//TEST:SIMPLE(filecheck=GLSLALLCHECK):-target glsl -entry computeAllMain -stage compute
//TEST:SIMPLE(filecheck=GLSLDEVCHECK):-target glsl -entry computeDevMain -stage compute
//TEST:SIMPLE(filecheck=GLSLGRPCHECK):-target glsl -entry computeGrpMain -stage compute

RWStructuredBuffer<uint> buf;
groupshared uint shared1[64];

[numthreads(64,1,1)]
void computeAllMain(uint3 tid : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    buf[tid.x] = tid.x;
    AllMemoryBarrier();
    buf[tid.x + 1] = buf[0];
}

[numthreads(64,1,1)]
void computeDevMain(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = tid.x;
    DeviceMemoryBarrier();
    buf[tid.x + 1] = buf[0];
}

[numthreads(64,1,1)]
void computeGrpMain(uint3 tid : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    shared1[gid.x] = tid.x;
    GroupMemoryBarrier();
    buf[tid.x] = shared1[63 - gid.x];
}

// All: semantics 2376 = SeqCst|UniformMemory|WorkgroupMemory|ImageMemory
// ALLCHECK: OpMemoryBarrier
// ALLCHECK-SAME: %uint_2376

// Device: semantics 2120 = SeqCst|UniformMemory|ImageMemory (no WorkgroupMemory)
// DEVCHECK: OpMemoryBarrier
// DEVCHECK-SAME: %uint_2120

// ThreadGroup: semantics 264 = SeqCst|WorkgroupMemory (no UniformMemory or ImageMemory)
// GRPCHECK: OpMemoryBarrier
// GRPCHECK-SAME: %uint_264

// Metal All: threadgroup_barrier with device+threadgroup+texture+imageblock flags.
// METALALLCHECK: threadgroup_barrier
// METALALLCHECK-SAME: mem_device

// Metal Device: same threadgroup_barrier flags (Metal has no device-only barrier surface).
// METALDEVCHECK: threadgroup_barrier
// METALDEVCHECK-SAME: mem_device

// Metal Group: threadgroup_barrier with mem_threadgroup flag only (shared memory scope).
// METALGRPCHECK: threadgroup_barrier
// METALGRPCHECK-SAME: mem_threadgroup

// WGSL All: storageBarrier + workgroupBarrier (all-memory coverage).
// WGSLALLCHECK: storageBarrier
// WGSLALLCHECK: workgroupBarrier

// WGSL Device: storageBarrier + workgroupBarrier (no device-only barrier in WGSL).
// WGSLDEVCHECK: storageBarrier

// WGSL Group: workgroupBarrier only (shared memory scope).
// WGSLGRPCHECK: workgroupBarrier

// CUDA All: __threadfence() (device-scope fence covering all memory).
// CUDAALLCHECK: __threadfence()

// CUDA Device: __threadfence() (device-scope fence for storage buffers and images).
// CUDADEVCHECK: __threadfence()

// CUDA Group: __threadfence_block() (block-scope shared memory fence).
// CUDAGRPCHECK: __threadfence_block()

// GLSL All: memoryBarrier with gl_ScopeDevice and shared+image+buffer semantics.
// GLSLALLCHECK: memoryBarrier
// GLSLALLCHECK-SAME: gl_ScopeDevice
// GLSLALLCHECK-SAME: gl_StorageSemanticsBuffer

// GLSL Device: memoryBarrier with gl_ScopeDevice and image+buffer semantics (no shared).
// GLSLDEVCHECK: memoryBarrier
// GLSLDEVCHECK-SAME: gl_ScopeDevice
// GLSLDEVCHECK-SAME: gl_StorageSemanticsBuffer

// GLSL Group: memoryBarrier with gl_ScopeWorkgroup and shared semantics only.
// GLSLGRPCHECK: memoryBarrier
// GLSLGRPCHECK-SAME: gl_ScopeWorkgroup
// GLSLGRPCHECK-SAME: gl_StorageSemanticsShared

// Targets not covered by emission directives:
//   cpp    - AllMemoryBarrier/DeviceMemoryBarrier/GroupMemoryBarrier not available on cpp target (E36107 capability gate).
