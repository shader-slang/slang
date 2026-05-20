//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#emit-dispatcher
//META: doc_section_digest=05e360846fc3ee11f7582f6896e802ffbb7981b023d417a3b600718f4844ea5b
//META: purpose=Boundary: when the same source has two compute entry points, compiling with `-entry foo` produces foo's body (and not bar's), and `-entry bar` produces bar's body (and not foo's). The dispatcher routes a single entry per compile.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the emit dispatcher: a single source file with
// two `[shader("compute")]` entry points produces different output
// depending on `-entry`. The two entry-point bodies write different
// constants (11 vs 22) and carry different [numthreads(...)] values
// (8 vs 16) so that the emit can be pinned to one or the other.

//TEST:SIMPLE(filecheck=FOO):-target hlsl -entry foo -stage compute
//TEST:SIMPLE(filecheck=BAR):-target hlsl -entry bar -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void foo(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 11;
}

[shader("compute")]
[numthreads(16, 1, 1)]
void bar(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 22;
}

// FOO: numthreads(8, 1, 1)
// FOO: void foo
// FOO-NOT: numthreads(16, 1, 1)
// FOO-NOT: void bar

// BAR: numthreads(16, 1, 1)
// BAR: void bar
// BAR-NOT: numthreads(8, 1, 1)
// BAR-NOT: void foo
