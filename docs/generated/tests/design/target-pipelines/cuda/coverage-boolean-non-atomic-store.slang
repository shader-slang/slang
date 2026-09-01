//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#option-set-toggles-and-emit-side-options
//META: doc_section_digest=50a146895994e911cc46818c5e9119d818f6d3745a677728ecde705d3636ba92
//META: purpose=slangc -trace-coverage-boolean forwards the boolean coverage flag to instrumentCoverage so each counter slot is written with a plain non-atomic store of 1 rather than atomically incremented per execution.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented difference between the two coverage
// counting modes at the level of the emitted CUDA statement. The
// same branching shader is compiled with the boolean flag added on
// top of -trace-coverage; the CHECK asserts the counter slot is
// assigned 1ULL directly and that no atomicAdd on the coverage
// buffer survives, which is what "removes all atomic contention"
// means concretely in the emitted text.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute -trace-coverage -trace-coverage-boolean

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

// A plain store of 1, not an atomic increment.
// CHECK: *(&(globalParams_{{[0-9]+}}->_slang_coverage_{{[0-9]+}})[int(0)]) = 1ULL;
// CHECK-NOT: atomicAdd((&(globalParams_{{[0-9]+}}->_slang_coverage_
