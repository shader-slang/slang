//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#barriers
//META: doc_section_digest=28e72df67f1e8f2f68fd59f23431c2be534ec39fa4206bc57dd04922591e5594
//META: purpose=`GroupMemoryBarrier()` emits `OpMemoryBarrier` with Workgroup scope and WorkgroupMemory semantics in SPIRV, covering only thread-group (shared) memory accesses.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `GroupMemoryBarrier()` emits `OpMemoryBarrier` with Workgroup scope (2)
// and semantics 264 (SeqCst | WorkgroupMemory), covering only groupshared memory.
// CHECK-SAME chains pin %uint_2 (Workgroup scope) and %uint_264; the absence of
// UniformMemory and ImageMemory bits distinguishes this from device-memory barriers.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSLCHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSLCHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALCHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSLCHECK):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDACHECK):-target cuda -entry computeMain -stage compute

groupshared uint sharedBuf[64];
RWStructuredBuffer<uint> buf;

[numthreads(64,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    sharedBuf[gid.x] = tid.x;
    GroupMemoryBarrier();
    buf[tid.x] = sharedBuf[63 - gid.x];
}

// SPIRV: OpMemoryBarrier Workgroup scope (2), thread-group semantics (264).
// CHECK: OpMemoryBarrier
// CHECK-SAME: %uint_2
// CHECK-SAME: %uint_264

// HLSL: preserves GroupMemoryBarrier intrinsic name.
// HLSLCHECK: GroupMemoryBarrier()

// GLSL: emits memoryBarrier with gl_ScopeWorkgroup and shared-storage semantics.
// GLSLCHECK: memoryBarrier
// GLSLCHECK-SAME: gl_ScopeWorkgroup
// GLSLCHECK-SAME: gl_StorageSemanticsShared

// Metal: emits threadgroup_barrier with mem_threadgroup flag (group-shared memory only).
// METALCHECK: threadgroup_barrier
// METALCHECK-SAME: mem_threadgroup

// WGSL: emits workgroupBarrier() (workgroup-scope shared memory barrier).
// WGSLCHECK: workgroupBarrier

// CUDA: emits __threadfence_block() (thread-block-scope shared memory fence).
// CUDACHECK: __threadfence_block()

// Targets not covered by emission directives:
//   cpp    - GroupMemoryBarrier is not available on cpp target (E36107 capability gate).
