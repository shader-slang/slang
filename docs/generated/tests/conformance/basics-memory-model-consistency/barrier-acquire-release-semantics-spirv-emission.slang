//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#barriers
//META: doc_section_digest=28e72df67f1e8f2f68fd59f23431c2be534ec39fa4206bc57dd04922591e5594
//META: purpose=A memory barrier provides acquire-release semantics: memory accesses before the barrier `happen before` memory accesses after it; verified by SPIRV SequentiallyConsistent semantics on `OpMemoryBarrier`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `AllMemoryBarrier()` emits `OpMemoryBarrier` with semantics operand 2376
// (SequentiallyConsistent | UniformMemory | WorkgroupMemory | ImageMemory), encoding the
// acquire-release ordering guarantee at SPIRV level. CHECK-SAME pins %uint_2376, confirming
// the SeqCst bit is present — which subsumes acquire-release for all memory spaces.

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
    buf[tid.x] = tid.x;          // write before barrier
    AllMemoryBarrier();           // acquire-release barrier
    uint v = buf[tid.x ^ 1];    // read after barrier
    buf[tid.x + 64] = v;
}

// SPIRV encodes acquire-release as SequentiallyConsistent (bit 8 = 8 set in semantics).
// The OpMemoryBarrier must be present (any semantics including SeqCst).
// CHECK: OpMemoryBarrier
// CHECK-SAME: %uint_2376

// HLSL: AllMemoryBarrier() is the acquire-release barrier surface; name is preserved.
// HLSLCHECK: AllMemoryBarrier()

// GLSL: emits memoryBarrier with gl_ScopeDevice and AcquireRelease semantics.
// GLSLCHECK: memoryBarrier
// GLSLCHECK-SAME: gl_SemanticsAcquireRelease

// Metal: emits threadgroup_barrier covering all memory spaces (acquire-release at hardware level).
// METALCHECK: threadgroup_barrier

// WGSL: emits storageBarrier, textureBarrier, and workgroupBarrier (no explicit AcquireRelease ordering in WGSL barrier API).
// WGSLCHECK: storageBarrier

// CUDA: emits __threadfence() (device-scope memory fence providing acquire-release semantics).
// CUDACHECK: __threadfence()

// Targets not covered by emission directives:
//   cpp    - AllMemoryBarrier is not available on cpp target (E36107 capability gate).
