//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#emit-dispatcher
//META: doc_section_digest=f136369c763a469783c526b150d8baf5d8f6435a816fbf58504eaa5c871b9d36
//META: purpose=The C++ and CUDA header target variants (`-target hpp`, `-target cuh`) are routed through the same textual emit path as their source variants and emit an include guard plus the same prelude.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the header variants of the C-family source targets belong to
// the textual set the dispatcher hands to the source-emit path. The same
// kernel is compiled to `hpp` and `cuh`; each CHECK asserts the output opens
// with `#pragma once` and carries the prelude header of the corresponding
// source language, which is only true if the header variant was mapped to the
// C++ / CUDA emitter rather than rejected as an unknown target.

//TEST:SIMPLE(filecheck=HPP):-target hpp -entry main -stage compute
//TEST:SIMPLE(filecheck=CUH):-target cuh -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) + 1;
}

// HPP: {{^}}#pragma once
// HPP: slang-cpp-prelude.h

// CUH: {{^}}#pragma once
// CUH-DAG: slang-cuda-prelude.h
// CUH-DAG: SLANG_globalParams
