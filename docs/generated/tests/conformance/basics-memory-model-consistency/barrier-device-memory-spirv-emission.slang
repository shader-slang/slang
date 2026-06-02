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
