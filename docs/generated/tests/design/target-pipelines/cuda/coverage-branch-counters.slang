//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=dc8998821e048cc2136a09100dd12b179d65f855a316bc0f1a8c26f8eaf6337d
//META: purpose=slangc -trace-branch-coverage is the third option that sets reqSet.coverageTracing, and it places one counter in each arm of a branch so a two-arm if/else gets two distinct coverage slots.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the branch-entry member of the documented trio of
// coverage options. The distinguishing observable against the
// function-entry sibling is the counter index: branch coverage
// allocates a slot per arm, so the CHECKs assert both slot 0 and
// slot 1 of the shared coverage buffer are incremented, one inside
// each arm of the if/else.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute -trace-branch-coverage

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

// CHECK: RWStructuredBuffer<ulonglong> _slang_coverage_{{[0-9]+}};
// CHECK: atomicAdd((&(globalParams_{{[0-9]+}}->_slang_coverage_{{[0-9]+}})[int(0)]), 1ULL)
// CHECK: atomicAdd((&(globalParams_{{[0-9]+}}->_slang_coverage_{{[0-9]+}})[int(1)]), 1ULL)
