//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:59:05+00:00
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/generated/design/architecture/dependency-graph.md#edges-intra-project-only
//META: doc_section_digest=9710717313ceae695df9e18bae5d2ef751a05dfd90122a44daa3fba34798a676
//META: purpose=The `slangLib -> prelude` private-include edge ships per-target prelude headers; CUDA emit literally `#include`s `slang-cuda-prelude.h`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the slangc-observable consequence of the
// `slangLib -.->|source include| prelude`-style edge documented in
// the dependency graph (in the per-edge citation table, prelude is
// listed as a "private include dep, not a static link"): the CUDA
// emit path literally `#include`s the prelude file shipped from
// `prelude/`, naming `slang-cuda-prelude.h` verbatim. If the edge
// were removed, the include would not appear in emitted CUDA text.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    //CHECK: slang-cuda-prelude.h
    outBuf[tid.x] = int(tid.x);
}
