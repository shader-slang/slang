//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:40:21+00:00
//META: source_commit=4e394f5c941b8145021611e1d3016b1f27a4ba52
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=53fa67e8e5b5b7cc0d09c737e54d406ba6f9a31c67e2efff85428bbfda70a405
//META: purpose=floatNonUniformResourceIndex runs for CUDA (gated only on !isSPIRV, with NonUniformResourceIndexFloatMode::Textual) so a NonUniformResourceIndex-wrapped dynamic buffer index still emits as a well-formed CUDA kernel with the indexed access intact.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that CUDA is on the floatNonUniformResourceIndex path
// (Phase C row 8: the pass fires for every non-SPIR-V target; only the
// narrower four-way legalizeLogicalAndOr gate excludes CUDA). The test
// wraps a thread-derived buffer index in NonUniformResourceIndex and
// asserts the kernel still emits with the indexed load/store surviving
// — i.e. the pass consumed the wrapper on CUDA instead of leaving an
// unlowered intrinsic behind. The doc does not pin a Textual-mode emit
// fingerprint for CUDA, so the CHECKs pin only the well-formed-kernel
// and surviving-access observations.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

StructuredBuffer<float> inBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint idx = NonUniformResourceIndex(tid.x);
    outBuf[idx] = inBuf[idx];
}

// CHECK: extern "C" __global__ void main
// CHECK: outBuf_{{[0-9]+}}
