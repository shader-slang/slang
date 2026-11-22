//META: generated=true
//META: model=gpt-5.6-sol
//META: generated_at=2026-09-16T06:40:19Z
//META: source_commit=d3b56927b854c14fa1ac169b72da6e080ea80833
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=176050d5aa6473a960364c4e8306dcb0207b369af4a0a0c828054dd949a1f62f
//META: purpose=Direct loads from top-level uniform fields are rooted in CUDA's `__constant__` `SLANG_globalParams` group and remain plain loads rather than `__ldg` global-memory reads.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A top-level uniform is immutable, but CUDA emits its synthesized
// parameter group in `__constant__` memory. `__ldg` lowers to the
// global-memory-only `ld.global.nc`, so this direct field read must
// remain plain.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

uniform uint gValue;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = gValue;
}

// CHECK: struct GlobalParams_0
// CHECK-NOT: __ldg
// CHECK: extern "C" __constant__ GlobalParams_0 SLANG_globalParams;
// CHECK: extern "C" __global__ void main_0()
// CHECK-NOT: __ldg
