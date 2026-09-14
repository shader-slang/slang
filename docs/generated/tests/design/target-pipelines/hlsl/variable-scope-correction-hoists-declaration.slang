//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#applyvariablescopecorrection
//META: doc_section_digest=e0c73172b7f0a7ebae58bc830e5fed61bdf7ce15ce95d1b6c823a081828527f3
//META: purpose=A value defined inside a loop and used after it surfaces in emitted HLSL as a local declaration lifted to the top of the function body, ahead of the loop that computes it.
//META: intent=functional
//META: pipeline_stage=legalize
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `applyVariableScopeCorrection` runs for every non-SPIR-V target. It repairs
// values defined inside a loop but used after it — a use is "out of scope" when
// its block is dominated by the loop's break block.
//
// Two of its three repairs share the insertion point
// `entryBlock->getFirstOrdinaryInst()`, and an `IRVar` is on the emitter's
// never-fold list, so both surface the same way: a declaration lifted to the
// top of the function body, ahead of the loop, with the loop body assigning
// into it and the post-loop use reading it back.
//
// There is no marker of any kind for the repair, so the only checkable
// signature is positional — the declaration sits before the `for`, not inside
// it. CHECK asserts that order; CHECK-NOT rules out the declaration appearing
// only within the loop body.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int acc = 0;
    int lastSeen;
    for (int i = 0; i < 4; ++i)
    {
        lastSeen = int(tid.x) + i;
        acc += lastSeen;
    }
    // `lastSeen` is defined inside the loop and read after it.
    outBuf[0] = acc + lastSeen;
}

// The repaired value is declared at the top of the function body, before the
// loop that assigns it.
// CHECK: void main
// CHECK: int {{[A-Za-z_0-9]+}};
// CHECK: for(
