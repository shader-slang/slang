//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
//META: purpose=Expansion: a Slang do-while loop is emitted as an HLSL loop with a tail conditional branch by the shared C-like emitter.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// do-while differs from while only in the IR ordering of the
// condition branch. The emitter still produces a valid HLSL loop.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int i = 0;
    int acc = 0;
    do
    {
        acc = acc + i;
        i = i + 1;
    } while (i < int(tid.x) + 1);
    outBuf[tid.x] = acc;
}

// CHECK: for(
