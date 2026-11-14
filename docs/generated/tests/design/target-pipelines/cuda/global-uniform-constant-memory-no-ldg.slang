//META: generated=true
//META: model=gpt-5.6-sol
//META: generated_at=2026-09-16T06:40:19Z
//META: source_commit=d3b56927b854c14fa1ac169b72da6e080ea80833
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=031d306ccbb9670357e2a8af6d7e786afcb208ea0c4a357ccf9a309fc8034205
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

// CHECK-NOT: __ldg
// CHECK: SLANG_globalParams
// CHECK-NOT: __ldg
