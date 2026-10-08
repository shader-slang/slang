//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#barriers
//META: doc_section_digest=28e72df67f1e8f2f68fd59f23431c2be534ec39fa4206bc57dd04922591e5594
//META: purpose=`DeviceMemoryBarrier()` emits `OpMemoryBarrier` with Device scope and device-memory-only semantics in SPIRV, covering only address spaces with all-threads instance scope.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `DeviceMemoryBarrier()` emits `OpMemoryBarrier` with Device scope (1)
// and semantics 2120 (SeqCst | UniformMemory | ImageMemory), covering storage buffers
// and images but not groupshared memory. The absence of WorkgroupMemory(256) in 2120
// (vs 2376 for AllMemoryBarrier) is what distinguishes the device-only scope.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSLCHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSLCHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALCHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSLCHECK):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDACHECK):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<uint> buf;

[numthreads(64,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = tid.x;
    DeviceMemoryBarrier();
    buf[tid.x + 1] = buf[0];
}

// SPIRV: OpMemoryBarrier Device scope (1), device-memory semantics (2120).
// CHECK: OpMemoryBarrier
// CHECK-SAME: %uint_1
// CHECK-SAME: %uint_2120

// HLSL: preserves DeviceMemoryBarrier intrinsic name.
// HLSLCHECK: DeviceMemoryBarrier()

// GLSL: emits memoryBarrier with gl_ScopeDevice and image+buffer storage semantics (no shared).
// GLSLCHECK: memoryBarrier
// GLSLCHECK-SAME: gl_ScopeDevice
// GLSLCHECK-SAME: gl_StorageSemanticsBuffer

// Metal: emits threadgroup_barrier with device, threadgroup, texture, and imageblock flags
// (Metal has no device-only barrier; threadgroup_barrier is the closest available form).
// METALCHECK: threadgroup_barrier
// METALCHECK-SAME: mem_device

// WGSL: emits storageBarrier, textureBarrier, and workgroupBarrier (WGSL has no device-only barrier).
// WGSLCHECK: storageBarrier
// WGSLCHECK: workgroupBarrier

// CUDA: emits __threadfence() (device-scope memory fence; covers storage buffers and images).
// CUDACHECK: __threadfence()

// Targets not covered by emission directives:
//   cpp    - DeviceMemoryBarrier is not available on cpp target (E36107 capability gate).
