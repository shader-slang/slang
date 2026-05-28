//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:59:05+00:00
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/generated/design/architecture/dependency-graph.md#edges-intra-project-only
//META: doc_section_digest=9710717313ceae695df9e18bae5d2ef751a05dfd90122a44daa3fba34798a676
//META: purpose=The `slangLib -> prelude` private-include edge ships per-target prelude headers; CPP emit literally `#include`s `slang-cpp-prelude.h`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the slangc-observable consequence of the same private-
// include edge as the CUDA case, on a different text-emit target:
// the C++ shader-output emit path literally `#include`s the
// `slang-cpp-prelude.h` header shipped from `prelude/`. Two distinct
// targets exercise the edge to catch a per-target regression where
// the include drops on one backend but not the other.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    //CHECK: slang-cpp-prelude.h
    outBuf[tid.x] = int(tid.x);
}
