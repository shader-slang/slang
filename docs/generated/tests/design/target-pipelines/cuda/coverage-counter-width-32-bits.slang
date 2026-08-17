//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#option-set-toggles-and-emit-side-options
//META: doc_section_digest=50a146895994e911cc46818c5e9119d818f6d3745a677728ecde705d3636ba92
//META: purpose=Boundary - slangc -trace-coverage-counter-width 32 stores the corresponding byte width, so the synthesized coverage buffer narrows from the default ulonglong counters to uint counters incremented by 1U.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower of the two accepted counter widths actually
// reaches emit rather than being silently ignored. The default
// (8-byte) width is pinned by the sibling coverage test as
// RWStructuredBuffer<ulonglong> with 1ULL increments; here the
// CHECK asserts the 32-bit request produces uint slots and a 1U
// increment instead, which is the observable consequence of the
// byte width being forwarded to instrumentCoverage.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute -trace-coverage -trace-coverage-counter-width 32

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = int(tid.x);
}

// CHECK: RWStructuredBuffer<uint> _slang_coverage_{{[0-9]+}};
// CHECK: atomicAdd((&(globalParams_{{[0-9]+}}->_slang_coverage_{{[0-9]+}})[int(0)]), 1U)
