//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=dc8998821e048cc2136a09100dd12b179d65f855a316bc0f1a8c26f8eaf6337d
//META: purpose=slangc -trace-coverage turns on instrumentCoverage, and on CUDA the synthesized coverage buffer is packed into the GlobalParams struct alongside the user's own buffers, with each coverage entry incremented by an atomicAdd.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the CUDA-specific half of the coverage-instrumentation
// claim: the buffer the pass synthesizes is not a loose global but
// a field of the same __constant__ GlobalParams block the user's
// uniforms land in, because entry-point uniform packing runs before
// the coverage metadata is finalized. The shader has a branch, so
// instrumentation has more than one entry to count. The CHECK
// asserts the coverage buffer appears as a GlobalParams field and
// that each entry is bumped with atomicAdd — the default counting
// mode, as opposed to the boolean mode covered by its own test.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute -trace-coverage

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
        outputBuffer[0] = 1;
    else
        outputBuffer[0] = 2;
}

// The synthesized buffer is a field of GlobalParams, next to the
// user's own buffer — not a separate global.
// CHECK: struct GlobalParams_{{[0-9]+}}
// CHECK: RWStructuredBuffer<int> outputBuffer_{{[0-9]+}};
// CHECK: RWStructuredBuffer<ulonglong> _slang_coverage_{{[0-9]+}};
// CHECK: __constant__ GlobalParams_{{[0-9]+}}

// Default (non-boolean) mode counts executions atomically.
// CHECK: atomicAdd((&(globalParams_{{[0-9]+}}->_slang_coverage_{{[0-9]+}})[int(0)]), 1ULL)
