//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#cuda
//META: doc_section_digest=aec099a690ba0cb7b3cba6485d62487754f84f320420d4d5113995c23ee1a4bb
//META: purpose=Boundary: a kernel with multiple module-scope uniform globals is emitted on CUDA via the GlobalParams struct + SLANG_globalParams shim (the `__constant__ GlobalParams_N SLANG_globalParams` pattern documented in the bundle's doc-gap note).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the CUDA emit shape under multiple module-scope
// uniforms: the CUDA backend collects them into a single
// `GlobalParams_N` struct and exposes them through the
// `SLANG_globalParams` constant memory shim. Stressing the count
// from one (covered by the existing `cuda-prelude-and-global.slang`)
// to three pins the struct's multi-field layout.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

uniform int x;
uniform int y;
uniform int z;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = x + y + z;
}

// CHECK: struct GlobalParams_{{[0-9]+}}
// CHECK: extern "C" __constant__ GlobalParams_{{[0-9]+}} SLANG_globalParams
// CHECK: extern "C" __global__ void main_{{[0-9]+}}()
